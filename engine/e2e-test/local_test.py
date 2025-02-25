import requests
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
import platform
import asyncio
import argparse
import websockets

# Define a list of request configurations
model_id = "tinyllama:1b"


def make_request(config):
    try:
        method = config["method"].lower()
        url = config["url"]
        headers = config.get("headers", {})
        data = json.dumps(config.get("data")) if "data" in config else None

        response = requests.request(method, url, headers=headers, data=data)
        return response.status_code, response.text
    except requests.RequestException as e:
        return None, str(e)


def get_setup_configs(host_port):
    return [
        {
            "method": "PATCH",
            "url": "http://" + host_port + "/v1/configs",
            "headers": {"Content-Type": "application/json"},
            "data": {
                "cors": True,
                "allowed_origins": [
                    "http://localhost:39281",
                    "http://127.0.0.1:39281",
                    "http://0.0.0.0:39281",
                ],
                "proxy_username": "",
                "proxy_password": "",
                "proxy_url": "",
                "verify_proxy_ssl": False,
                "verify_proxy_host_ssl": False,
                "verify_peer_ssl": False,
                "verify_host_ssl": False,
                "no_proxy": "localhost",
                "huggingface_token": "",
            },
        },
        {
            "method": "POST",
            "url": "http://" + host_port + "/v1/engines/llama-cpp/install",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "GET",
            "url": "http://" + host_port + "/v1/engines/llama-cpp",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "GET",
            "url": "http://" + host_port + "/v1/engines/llama-cpp/releases",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "GET",
            "url": "http://" + host_port + "/v1/engines/llama-cpp/releases/latest",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "GET",
            "url": "http://" + host_port + "/v1/engines/llama-cpp/default",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "POST",
            "url": "http://" + host_port + "/v1/models/pull",
            "headers": {"Content-Type": "application/json"},
            "data": {"model": "tinyllama:1b"},
        },
        {
            "method": "POST",
            "url": "http://" + host_port + "/v1/engines/llama-cpp/load",
            "headers": {"Content-Type": "application/json"},
        },
        {
            "method": "POST",
            "url": "http://" + host_port + "/v1/models/start",
            "headers": {"Content-Type": "application/json"},
            "data": {"model": "tinyllama:1b"},
        },
        {
            "method": "POST",
            "url": "http://" + host_port + "/v1/chat/completions",
            "headers": {"Content-Type": "application/json"},
            "data": {
                "model": "tinyllama:1b",
                "stream": True,
                "messages": [{"content": "How are you today?", "role": "user"}],
                "max_tokens": 256,
            },
        },
    ]


def get_teardown_configs(host_port):
    return [
        {
            "method": "POST",
            "url": "http://" + host_port + "/v1/models/stop",
            "headers": {"Content-Type": "application/json"},
            "data": {"model": "tinyllama:1b"},
        },
        {
            "method": "DELETE",
            "url": "http://" + host_port + "/v1/engines/llama-cpp/load",
            "headers": {"Content-Type": "application/json"},
        },
        {
            "method": "DELETE",
            "url": "http://" + host_port + "/v1/engines/llama-cpp/install",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "DELETE",
            "url": "http://" + host_port + "/v1/models/" + model_id,
            "headers": {"Accept": "application/json"},
        },
    ]


async def setup_env(host_port):
    for config in get_setup_configs(host_port):
        status_code, response_text = make_request(config)
        if config["method"] == "POST" and (
            "/v1/engines/install" in config["url"] or "/v1/models/pull" in config["url"]
        ):
            await wait_for_websocket_download_success_event(timeout=None)

        if status_code:
            print(
                f"setup_env: {config['url']} Status Code {status_code} - Response {response_text}"
            )
        else:
            print(f"setup_env: {config['url']} Error - {response_text}")


def teardown(host_port):
    for config in get_teardown_configs(host_port):
        status_code, response_text = make_request(config)

        if status_code:
            print(f"teardown: {config['url']} Status Code {status_code}")
        else:
            print(f"teardown: {config['url']} Error - {response_text}")


def get_request_configs(host_port):
    return [
        {
            "method": "GET",
            "url": "http://" + host_port + "/v1/configs",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "GET",
            "url": "http://" + host_port + "/v1/models",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "GET",
            "url": "http://" + host_port + "/v1/hardware",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "GET",
            "url": "http://" + host_port + "/healthz",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "GET",
            "url": "http://" + host_port + "/v1/threads",
            "headers": {"Accept": "application/json"},
        },
        {
            "method": "GET",
            "url": "http://" + host_port + "/v1/threads",
            "headers": {"Accept": "application/json"},
        },
    ]


def worker(host_port, thread_id, num_requests):
    request_configs = get_request_configs(host_port)
    for i in range(num_requests):
        config = request_configs[i % len(request_configs)]
        status_code, response_text = make_request(config)
        if status_code:
            print(
                f"Thread {thread_id}, Request {i+1}: {config['method']} {config['url']} - Status Code {status_code}"
            )
        else:
            print(
                f"Thread {thread_id}, Request {i+1}: {config['method']} {config['url']} - Error - {response_text}"
            )


async def wait_for_websocket_download_success_event(timeout: float = 30):
    if platform.system() == "Windows":
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    async with websockets.connect("ws://" + host_port + "/events") as websocket:
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


def run_test(host_port, num_threads, requests_per_thread):
    start_time = time.time()

    with ThreadPoolExecutor(max_workers=num_threads) as executor:
        futures = [
            executor.submit(worker, host_port, i, requests_per_thread)
            for i in range(num_threads)
        ]

        for future in as_completed(futures):
            future.result()

    end_time = time.time()
    total_requests = num_threads * requests_per_thread
    total_time = end_time - start_time

    print(f"\nTest completed:")
    print(f"Total threads: {num_threads}")
    print(f"Requests per thread: {requests_per_thread}")
    print(f"Total requests: {total_requests}")
    print(f"Total time: {total_time:.2f} seconds")
    print(f"Requests per second: {total_requests / total_time:.2f}")


def parse_argument():
    parser = argparse.ArgumentParser(description="Local test")
    parser.add_argument("--host", type=str, default="127.0.0.1", help="Server host")
    parser.add_argument("--port", type=int, help="Server port", required=True)
    parser.add_argument(
        "--num_threads", type=str, default=5, help="Number of threads to send requests"
    )
    parser.add_argument(
        "--requests_per_thread",
        type=str,
        default=10,
        help="Number of requests per thread",
    )
    args = parser.parse_args()
    return args


if __name__ == "__main__":
    args = parse_argument()
    host_port = args.host + ":" + str(args.port)
    print("Server start: " + host_port)
    teardown(host_port)

    loop = asyncio.get_event_loop()
    loop.run_until_complete(setup_env(host_port))

    run_test(host_port, args.num_threads, args.requests_per_thread)

    teardown(host_port)
