import asyncio
import json
import platform
import queue
import select
import subprocess
import threading
import time
import requests
from requests.exceptions import RequestException
from typing import List

import websockets

# You might want to change the path of the executable based on your build directory
executable_windows_path = "build\\cortex.exe"
executable_unix_path = "build/cortex"

# Timeout
timeout = 5  # secs
start_server_success_message = "Server started"


# Get the executable path based on the platform
def getExecutablePath() -> str:
    if platform.system() == "Windows":
        return executable_windows_path
    else:
        return executable_unix_path


# Execute a command
def run(test_name: str, arguments: List[str], timeout=timeout, capture = True) -> (int, str, str):
    executable_path = getExecutablePath()
    print("Running:", test_name)
    print("Command:", [executable_path] + arguments)

    result = subprocess.run(
        [executable_path] + arguments,
        capture_output=capture,
        text=True,
        timeout=timeout,
    )
    return result.returncode, result.stdout, result.stderr


def popen(arguments: List[str], user_input: str) -> (int, str, str):
    # Start the process
    executable_path = getExecutablePath()
    process = subprocess.Popen(
        [executable_path] + arguments,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,  # This ensures the input and output are treated as text
    )

    # Send input and get output
    stdout, stderr = process.communicate(input=user_input)

    # Get the return code
    return_code = process.returncode

    return stdout, stderr, return_code


# Start the API server
# Wait for `Server started` message or failed
def start_server() -> bool:
    if platform.system() == "Windows":
        return start_server_windows()
    else:
        return start_server_nix()


def start_server_if_needed():
    """
    Start the server if it is not already running.
    Sending a healthz request to the server to check if it is running.
    """
    try:
        response = requests.get(
            'http://localhost:3928/healthz',
            timeout=5
        )
        if response.status_code == 200:
            print("Server is already running")
    except RequestException as e:
        print("Server is not running. Starting the server...")
        start_server()
        

def pull_model_if_needed(model_id: str = "tinyllama:1b"):
    """
    Pull the model if it is not already pulled.
    """
    should_pull = False
    try:
        response = requests.get("http://localhost:3928/models/" + model_id,
                                timeout=5
        )
        if response.status_code != 200:
            should_pull = True

    except RequestException as e:
        print("Http error occurred: " + e)
    
    if should_pull:
        run("Pull model", ["pull", model_id], timeout=10*60)


def start_server_nix() -> bool:
    executable = getExecutablePath()
    process = subprocess.Popen(
        [executable] + ["start", "--port", "3928"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    start_time = time.time()
    while time.time() - start_time < timeout:
        # Use select to check if there's data to read from stdout or stderr
        readable, _, _ = select.select([process.stdout, process.stderr], [], [], 0.1)

        for stream in readable:
            line = stream.readline()
            if line:
                if start_server_success_message in line:
                    # have to wait a bit for server to really up and accept connection
                    print("Server started found, wait 0.3 sec..")
                    time.sleep(0.3)
                    return True

        # Check if the process has ended
        if process.poll() is not None:
            return False

    return False


def start_server_windows() -> bool:
    executable = getExecutablePath()
    process = subprocess.Popen(
        [executable] + ["start", "--port", "3928"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
        universal_newlines=True,
    )

    q_out = queue.Queue()
    q_err = queue.Queue()

    def enqueue_output(out, queue):
        for line in iter(out.readline, b""):
            queue.put(line)
        out.close()

    # Start threads to read stdout and stderr
    t_out = threading.Thread(target=enqueue_output, args=(process.stdout, q_out))
    t_err = threading.Thread(target=enqueue_output, args=(process.stderr, q_err))
    t_out.daemon = True
    t_err.daemon = True
    t_out.start()
    t_err.start()

    # only wait for defined timeout
    start_time = time.time()
    while time.time() - start_time < timeout:
        # Check stdout
        try:
            line = q_out.get_nowait()
        except queue.Empty:
            pass
        else:
            print(f"STDOUT: {line.strip()}")
            if start_server_success_message in line:
                return True

        # Check stderr
        try:
            line = q_err.get_nowait()
        except queue.Empty:
            pass
        else:
            print(f"STDERR: {line.strip()}")
            if start_server_success_message in line:
                # found the message. let's wait for some time for the server successfully started
                time.sleep(0.3)
                return True, process

        # Check if the process has ended
        if process.poll() is not None:
            return False

        time.sleep(0.1)

    return False


# Stop the API server
def stop_server():
    run("Stop server", ["stop"])


async def wait_for_websocket_download_success_event(timeout: float = 30):
    if platform.system() == 'Windows':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    async with websockets.connect("ws://127.0.0.1:3928/events") as websocket:
        try:
            # Using wait_for instead of timeout context manager
            async def receive_until_success():
                while True:
                    message = await websocket.recv()
                    try:
                        event = json.loads(message)
                        if event.get("type") == "DownloadSuccess":
                            return event
                    except json.JSONDecodeError:
                        continue

            return await asyncio.wait_for(receive_until_success(), timeout)

        except asyncio.TimeoutError:
            raise TimeoutError("Timeout waiting for DownloadSuccess event")


def get_latest_pre_release_tag(repo_owner, repo_name):
    # URL for GitHub API to fetch all releases of the repository
    url = f"https://api.github.com/repos/{repo_owner}/{repo_name}/releases"
    
    # Headers to specify the API version
    headers = {
        "Accept": "application/vnd.github.v3+json"
    }
    
    # Send a GET request to the GitHub API
    response = requests.get(url, headers=headers)
    
    # Check the response status; raise an error if the request failed
    if response.status_code != 200:
        raise Exception(f"Failed to fetch releases: {response.status_code}, {response.text}")
    
    # Parse the JSON response into a list of releases
    releases = response.json()
    
    # Filter the releases to include only pre-releases
    pre_releases = [release for release in releases if release.get("prerelease")]
    
    # If no pre-releases are found, raise an exception
    if not pre_releases:
        raise Exception("No pre-releases found")
    
    # Sort the pre-releases by creation date, newest first
    pre_releases.sort(key=lambda x: x["created_at"], reverse=True)
    
    # Return the tag name of the latest pre-release
    return pre_releases[0]["tag_name"]