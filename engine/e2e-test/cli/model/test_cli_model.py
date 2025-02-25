import pytest
import requests
import os
from pathlib import Path
from utils.test_runner import (
    run,
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)

class TestCliModel:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        # Clean up
        run("Delete model", ["models", "delete", "tinyllama:1b"])
        stop_server()
        
    def test_model_pull_with_direct_url_should_be_success(self):
        exit_code, output, error = run(
            "Pull model",
            [
                "pull",
                "https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v0.3-GGUF/blob/main/tinyllama-1.1b-chat-v0.3.Q2_K.gguf",
            ],
            timeout=None, capture=False
        )
        root = Path.home()
        assert os.path.exists(root / "cortexcpp" / "models" / "huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v0.3-GGUF/tinyllama-1.1b-chat-v0.3.Q2_K.gguf")
        assert exit_code == 0, f"Model pull failed with error: {error}"
        
    @pytest.mark.asyncio
    async def test_models_delete_should_be_successful(self):
        json_body = {"model": "tinyllama:1b"}
        response = requests.post("http://localhost:3928/v1/models/pull", json=json_body)
        assert response.status_code == 200, f"Failed to pull model: tinyllama:1b"
        await wait_for_websocket_download_success_event(timeout=None)

        exit_code, output, error = run(
            "Delete model", ["models", "delete", "tinyllama:1b"]
        )
        assert "Model tinyllama:1b deleted successfully" in output
        assert exit_code == 0, f"Model does not exist: {error}"