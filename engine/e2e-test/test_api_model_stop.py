import pytest
import requests
from test_runner import run, start_server, stop_server


class TestApiModelStop:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        stop_server()
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        run("Install engine", ["engines", "install", "llama-cpp"], 5 * 60)
        yield

        run("Uninstall engine", ["engines", "uninstall", "llama-cpp"])
        # Teardown
        stop_server()

    def test_models_stop_should_be_successful(self):
        json_body = {"model": "tinyllama:gguf"}
        response = requests.post(
            "http://localhost:3928/v1/models/start", json=json_body
        )
        assert response.status_code == 200, f"status_code: {response.status_code}"
        response = requests.post("http://localhost:3928/v1/models/stop", json=json_body)
        assert response.status_code == 200, f"status_code: {response.status_code}"
