import platform
import os
import pytest, requests
from utils.test_runner import run
from utils.test_runner import start_server, stop_server


class TestCliServerStart:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_server_start_cli_run_successfully(self):
        response = requests.get("http://localhost:3928/healthz")
        assert response.status_code == 200
