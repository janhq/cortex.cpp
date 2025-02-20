import pytest
import requests
from utils.test_runner import (
    run,
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)


class TestCliEngineUninstall:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        stop_server()
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        stop_server()

    @pytest.mark.asyncio
    async def test_engines_uninstall_llamacpp_should_be_successfully(self):
        response = requests.post("http://localhost:3928/v1/engines/llama-cpp/install")
        await wait_for_websocket_download_success_event(timeout=None)
        exit_code, output, error = run(
            "Uninstall engine", ["engines", "uninstall", "llama-cpp"]
        )
        assert "Engine llama-cpp uninstalled successfully!" in output
        assert exit_code == 0, f"Install engine failed with error: {error}"
