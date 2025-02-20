import pytest
import requests
from utils.test_runner import start_server, stop_server


class TestApiEngineList:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        # Not sure why but on macOS amd, the first start server timeouts with CI
        start_server()
        stop_server()
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_engines_list_api_run_successfully(self):
        response = requests.get("http://localhost:3928/engines")
        assert response.status_code == 200