import platform
import select
import subprocess
import time
from typing import List


def run(name: str, arguments: List[str]):
    if platform.system() == "Windows":
        executable = "build\\cortex-cpp.exe"
    else:
        executable = "build/cortex-cpp"
    print("Command name", name)
    print("Running command: ", [executable] + arguments)
    if len(arguments) == 0:
        result = subprocess.run(executable, capture_output=True, text=True, timeout=5)
    else:
        result = subprocess.run(
            [executable] + arguments, capture_output=True, text=True, timeout=5
        )
    return result.returncode, result.stdout, result.stderr


def start_server(timeout=5):
    if platform.system() == "Windows":
        executable = "build\\cortex-cpp.exe"
    else:
        executable = "build/cortex-cpp"
    process = subprocess.Popen(
        executable, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )

    success_message = "Server started"
    start_time = time.time()
    while time.time() - start_time < timeout:
        # Use select to check if there's data to read from stdout or stderr
        readable, _, _ = select.select([process.stdout, process.stderr], [], [], 0.1)

        for stream in readable:
            line = stream.readline()
            if line:
                print(line.strip())  # Print output for debugging
                if success_message in line:
                    # have to wait a bit for server to really up and accept connection
                    time.sleep(0.3)
                    return True, process  # Success condition met

        # Check if the process has ended
        if process.poll() is not None:
            return False, process  # Process ended without success message

    return False, process  # Timeout reached


def stop_server():
    run("Stop server", ["stop"])
