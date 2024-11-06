import pytest
import requests
from test_runner import start_server, stop_server


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
        response = requests.post("http://localhost:3928/v1/engines/llama-cpp")
        assert response.status_code == 200

    def test_engines_install_llamacpp_specific_version_and_variant(self):
        response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp?version=v0.1.35-27.10.24&variant=linux-amd64-avx-cuda-11-7"
        )
        assert response.status_code == 200

    def test_engines_install_llamacpp_specific_version_and_null_variant(self):
        response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp?version=v0.1.35-27.10.24"
        )
        assert response.status_code == 200
