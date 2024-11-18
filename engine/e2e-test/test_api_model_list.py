import pytest
import requests
from test_runner import start_server, stop_server


class TestApiModelList:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_models_list_should_be_successful(self):
        response = requests.get("http://localhost:3928/v1/models")
        assert response.status_code == 200
