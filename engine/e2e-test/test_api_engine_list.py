import pytest
import requests
from test_runner import start_server, stop_server


class TestApiEngineList:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        start_server()

        yield

        # Teardown
        stop_server()

    def test_engines_list_api_run_successfully(self):
        response = requests.get("http://localhost:3928/engines")
        assert response.status_code == 200
