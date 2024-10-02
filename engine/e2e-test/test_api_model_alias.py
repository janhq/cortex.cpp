import pytest
import requests
from test_runner import popen, run
from test_runner import start_server, stop_server


class TestApiModelAlias:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        stop_server()

    def test_models_set_alias_should_be_successful(self):
        body_json = {'model': 'tinyllama:gguf',
                     'modelAlias': 'tg'}
        response = requests.post("http://localhost:3928/models/alias", json = body_json)        
        assert response.status_code == 200
