import pytest
import requests
from test_runner import start_server, stop_server


class TestApiEngineUninstall:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_engines_uninstall_llamacpp_should_be_successful(self):
        response = requests.delete("http://localhost:3928/engines/cortex.llamacpp")
        assert response.status_code == 200
