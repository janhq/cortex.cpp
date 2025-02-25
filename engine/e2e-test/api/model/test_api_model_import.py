import pytest
import requests
from utils.test_runner import start_server, stop_server

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
        body_json = {'model': 'tinyllama:1b',
                     'modelPath': '/path/to/local/gguf'}
        response = requests.post("http://localhost:3928/v1/models/import", json=body_json)              
        assert response.status_code == 200

    @pytest.mark.skipif(True, reason="Expensive test. Only test when you have local gguf file.")
    def test_model_import_with_name_should_be_success(self):
        body_json = {'model': 'tinyllama:1b',
                     'modelPath': '/path/to/local/gguf',
                     'name': 'test_model'}
        response = requests.post("http://localhost:3928/v1/models/import", json=body_json)
        assert response.status_code == 200

    @pytest.mark.skipif(True, reason="Expensive test. Only test when you have local gguf file.")
    def test_model_import_with_name_should_be_success(self):
        body_json = {'model': 'testing-model',
                     'modelPath': '/path/to/local/gguf',
                     'name': 'test_model',
                     'option': 'copy'}
        response = requests.post("http://localhost:3928/v1/models/import", json=body_json)
        assert response.status_code == 200
        # Test imported path
        response = requests.get("http://localhost:3928/v1/models/testing-model")
        assert response.status_code == 200
        # Since this is a dynamic test - require actual file path
        # it's not safe to assert with the gguf file name
        assert response.json()['files'][0] != '/path/to/local/gguf'

    def test_model_import_with_invalid_path_should_fail(self):
        body_json = {'model': 'tinyllama:1b',
                     'modelPath': '/invalid/path/to/gguf'}
        response = requests.post("http://localhost:3928/v1/models/import", json=body_json)
        assert response.status_code == 400

    def test_model_import_with_missing_model_should_fail(self):
        body_json = {'modelPath': '/path/to/local/gguf'}
        response = requests.post("http://localhost:3928/v1/models/import", json=body_json)
        print(response)
        assert response.status_code == 409