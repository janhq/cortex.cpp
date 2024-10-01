import pytest
import requests
from test_runner import start_server, stop_server

class TestApiModelImport:
    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        stop_server()

    @pytest.mark.skipif(True, reason="Expensive test. Only test when you have local gguf file.")
    def test_model_import_should_be_success(self):
        body_json = {'modelId': 'tinyllama:gguf',
                     'modelPath': '/path/to/local/gguf'}
        response = requests.post("http://localhost:3928/models/import", json = body_json)              
        assert response.status_code == 200      