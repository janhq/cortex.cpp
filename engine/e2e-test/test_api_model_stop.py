import pytest
import requests
from test_runner import start_server, stop_server


class TestApiModelStop:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        requests.post("http://localhost:3928/engines/llama-cpp")
        yield

        requests.delete("http://localhost:3928/engines/llama-cpp")
        # Teardown
        stop_server()

    def test_models_stop_should_be_successful(self):
        json_body = {"model": "tinyllama:gguf"}
        response = requests.post("http://localhost:3928/models/start", json=json_body)
        assert response.status_code == 200, f"status_code: {response.status_code}"
        response = requests.post("http://localhost:3928/models/stop", json=json_body)
        assert response.status_code == 200, f"status_code: {response.status_code}"
