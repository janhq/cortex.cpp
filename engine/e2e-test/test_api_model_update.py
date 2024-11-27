import pytest
import requests
from test_runner import popen, run
from test_runner import start_server, stop_server


class TestApiModelUpdate:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        stop_server()

    def test_models_update_should_be_successful(self):
        body_json = {'model': 'tinyllama:gguf'}
        response = requests.patch("http://localhost:3928/v1/models/tinyllama:gguf", json = body_json)        
        assert response.status_code == 200
