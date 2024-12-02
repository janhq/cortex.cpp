from test_runner import run
from test_runner import start_server, stop_server
import os
from pathlib import Path

class TestCliModelPullDirectUrl:

    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
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
        # TODO: verify that the model has been pull successfully
        # TODO: skip this test. since download model is taking too long

