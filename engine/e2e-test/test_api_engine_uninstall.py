import pytest
import requests
from test_runner import start_server, stop_server


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

    def test_engines_uninstall_llamacpp_should_be_successful(self):
        # install first
        requests.post("http://localhost:3928/v1/engines/llama-cpp")

        response = requests.delete("http://localhost:3928/v1/engines/llama-cpp")
        assert response.status_code == 200

    def test_engines_uninstall_llamacpp_with_only_version_should_be_failed(self):
        # install first
        install_response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp?version=v0.1.35"
        )
        assert install_response.status_code == 200

        response = requests.delete(
            "http://localhost:3928/v1/engines/llama-cpp?version=v0.1.35"
        )
        assert response.status_code == 400
        assert response.json()["message"] == "No variant provided"

    def test_engines_uninstall_llamacpp_with_variant_should_be_successful(self):
        # install first
        install_response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp?variant=mac-arm64"
        )
        assert install_response.status_code == 200

        response = requests.delete(
            "http://localhost:3928/v1/engines/llama-cpp?variant=mac-arm64"
        )
        assert response.status_code == 200

    def test_engines_uninstall_llamacpp_with_specific_variant_and_version_should_be_successful(
        self,
    ):
        # install first
        install_response = requests.post(
            "http://localhost:3928/v1/engines/llama-cpp?variant=mac-arm64&version=v0.1.35"
        )
        assert install_response.status_code == 200

        response = requests.delete(
            "http://localhost:3928/v1/engines/llama-cpp?variant=mac-arm64&version=v0.1.35"
        )
        assert response.status_code == 200
