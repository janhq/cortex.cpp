import pytest
import requests
from utils.test_runner import start_server, stop_server, get_latest_pre_release_tag

latest_pre_release_tag = get_latest_pre_release_tag("janhq", "cortex.llamacpp")

class TestApiEngineInstall:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_engines_install_llamacpp_should_be_successful(self):
        response = requests.post("http://localhost:3928/v1/engines/llama-cpp/install")
        assert response.status_code == 200

    def test_engines_install_llamacpp_specific_version_and_variant(self):
        data = {"version": latest_pre_release_tag, "variant": "linux-amd64-avx"}
        response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp/install", json=data
        )
        assert response.status_code == 200

    def test_engines_install_llamacpp_specific_version_and_null_variant(self):
        data = {"version": latest_pre_release_tag}
        response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp/install", json=data
        )
        assert response.status_code == 200
