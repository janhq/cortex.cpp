import pytest
import requests
from utils.test_runner import (
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)


class TestApiEngineUpdate:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")
        requests.delete("http://localhost:3928/v1/engines/llama-cpp")

        yield
        requests.delete("http://localhost:3928/v1/engines/llama-cpp")

        # Teardown
        stop_server()

    @pytest.mark.asyncio
    async def test_engines_update_should_be_successfully(self):
        requests.post("http://localhost:3928/v1/engines/llama-cpp?version=0.1.34")
        response = requests.post("http://localhost:3928/v1/engines/llama-cpp/update")
        assert response.status_code == 200

    @pytest.mark.asyncio
    async def test_engines_update_llamacpp_should_be_failed_if_already_latest(self):
        requests.post("http://localhost:3928/v1/engines/llama-cpp")
        await wait_for_websocket_download_success_event(timeout=None)
        get_engine_response = requests.get("http://localhost:3928/v1/engines/llama-cpp")
        assert len(get_engine_response.json()) > 0, "Response list should not be empty"

        response = requests.post("http://localhost:3928/v1/engines/llama-cpp/update")
        assert (
            "already up-to-date" in response.json()["message"]
        ), "Should display error message"
        assert response.status_code == 400
