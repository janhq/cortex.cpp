import pytest
import requests
from test_runner import (
    run,
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)


class TestCliModelDelete:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        # Clean up
        run("Delete model", ["models", "delete", "tinyllama:gguf"])
        stop_server()

    @pytest.mark.asyncio
    async def test_models_delete_should_be_successful(self):
        json_body = {"model": "tinyllama:gguf"}
        response = requests.post("http://localhost:3928/v1/models/pull", json=json_body)
        assert response.status_code == 200, f"Failed to pull model: tinyllama:gguf"
        await wait_for_websocket_download_success_event(timeout=None)

        exit_code, output, error = run(
            "Delete model", ["models", "delete", "tinyllama:gguf"]
        )
        assert "Model tinyllama:gguf deleted successfully" in output
        assert exit_code == 0, f"Model does not exist: {error}"
