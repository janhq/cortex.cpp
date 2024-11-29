import pytest
import time
import requests
from test_runner import (
    run,
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)


class TestApiEngineUninstall:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()
        
    @pytest.mark.asyncio
    async def test_engines_uninstall_llamacpp_should_be_successful(self):
        response = requests.post("http://localhost:3928/v1/engines/llama-cpp/install")
        assert response.status_code == 200
        await wait_for_websocket_download_success_event(timeout=None)
        time.sleep(30)
        
        response = requests.delete("http://localhost:3928/v1/engines/llama-cpp/install")
        assert response.status_code == 200

    @pytest.mark.asyncio
    async def test_engines_uninstall_llamacpp_with_only_version_should_be_failed(self):
        # install first
        data = {"variant": "mac-arm64"}
        install_response = requests.post(
            "http://127.0.0.1:3928/v1/engines/llama-cpp/install", json=data
        )
        await wait_for_websocket_download_success_event(timeout=120)
        assert install_response.status_code == 200

        data = {"version": "v0.1.35"}
        response = requests.delete(
            "http://localhost:3928/v1/engines/llama-cpp/install", json=data
        )
        assert response.status_code == 400
        assert response.json()["message"] == "No variant provided"

    @pytest.mark.asyncio
    async def test_engines_uninstall_llamacpp_with_variant_should_be_successful(self):
        # install first
        data = {"variant": "mac-arm64"}
        install_response = requests.post(
            "http://127.0.0.1:3928/v1/engines/llama-cpp/install", json=data
        )
        await wait_for_websocket_download_success_event(timeout=120)
        assert install_response.status_code == 200

        response = requests.delete("http://127.0.0.1:3928/v1/engines/llama-cpp/install")
        assert response.status_code == 200

    def test_engines_uninstall_llamacpp_with_specific_variant_and_version_should_be_successful(
        self,
    ):
        data = {"variant": "mac-arm64", "version": "v0.1.35"}
        # install first
        install_response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp/install", json=data
        )
        assert install_response.status_code == 200

        response = requests.delete(
            "http://localhost:3928/v1/engines/llama-cpp/install", json=data
        )
        assert response.status_code == 200
