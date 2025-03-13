import pytest
import requests
import time
from utils.test_runner import (
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)

class TestApiEngine:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    # engines get
    def test_engines_get_llamacpp_should_be_successful(self):
        response = requests.get("http://localhost:3928/engines/llama-cpp")
        assert response.status_code == 200

    # engines install
    def test_engines_install_llamacpp_specific_version_and_variant(self):
        data = {"version": "v0.1.40-b4354", "variant": "linux-amd64-avx"}
        response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp/install", json=data
        )
        assert response.status_code == 200

    def test_engines_install_llamacpp_specific_version_and_null_variant(self):
        data = {"version": "v0.1.40-b4354"}
        response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp/install", json=data
        )
        assert response.status_code == 200

    # engines uninstall
    @pytest.mark.asyncio
    async def test_engines_install_uninstall_llamacpp_should_be_successful(self):
        response = requests.post("http://localhost:3928/v1/engines/llama-cpp/install")
        assert response.status_code == 200
        await wait_for_websocket_download_success_event(timeout=None)
        time.sleep(30)

        response = requests.delete("http://localhost:3928/v1/engines/llama-cpp/install")
        assert response.status_code == 200

    @pytest.mark.asyncio
    async def test_engines_install_uninstall_python_should_be_successful(self):
        response = requests.post("http://localhost:3928/v1/engines/python-engine/install")
        assert response.status_code == 200
        await wait_for_websocket_download_success_event(timeout=None)
        time.sleep(30)

        response = requests.delete("http://localhost:3928/v1/engines/python-engine/install")
        assert response.status_code == 200

    @pytest.mark.asyncio
    async def test_engines_install_uninstall_llamacpp_with_only_version_should_be_failed(self):
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
    async def test_engines_install_uninstall_llamacpp_with_variant_should_be_successful(self):
        # install first
        data = {"variant": "mac-arm64"}
        install_response = requests.post(
            "http://127.0.0.1:3928/v1/engines/llama-cpp/install", json=data
        )
        await wait_for_websocket_download_success_event(timeout=120)
        assert install_response.status_code == 200

        response = requests.delete("http://127.0.0.1:3928/v1/engines/llama-cpp/install")
        assert response.status_code == 200

    def test_engines_install_uninstall_llamacpp_with_specific_variant_and_version_should_be_successful(
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

    