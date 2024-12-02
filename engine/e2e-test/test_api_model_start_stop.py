import time

import pytest
import requests
from test_runner import (
    run,
    start_server_if_needed,
    stop_server,
    wait_for_websocket_download_success_event,
)


class TestApiModelStartStop:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        start_server_if_needed()
        run("Delete model", ["models", "delete", "tinyllama:gguf"])

        yield

        # Teardown
        stop_server()

    @pytest.mark.asyncio
    async def test_models_start_should_be_successful(self):
        response = requests.post("http://localhost:3928/v1/engines/llama-cpp/install")
        assert response.status_code == 200
        await wait_for_websocket_download_success_event(timeout=None)
        # TODO(sang) need to fix for cuda download
        time.sleep(30)

        json_body = {"model": "tinyllama:gguf"}
        response = requests.post("http://localhost:3928/v1/models/pull", json=json_body)
        assert response.status_code == 200, f"Failed to pull model: tinyllama:gguf"
        await wait_for_websocket_download_success_event(timeout=None)

        json_body = {"model": "tinyllama:gguf"}
        response = requests.post(
            "http://localhost:3928/v1/models/start", json=json_body
        )
        assert response.status_code == 200, f"status_code: {response.status_code}"

        response = requests.post("http://localhost:3928/v1/models/stop", json=json_body)
        assert response.status_code == 200, f"status_code: {response.status_code}"
