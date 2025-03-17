import pytest
from utils.test_runner import popen


class TestCliModelPullHuggingFaceRepository:

    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()
    
    def test_model_pull_hugging_face_repository(self):
        """
        Test pull model pervll/bge-reranker-v2-gemma-Q4_K_M-GGUF from issue #1017
        """

        stdout, stderr, return_code = popen(
            ["pull", "pervll/bge-reranker-v2-gemma-Q4_K_M-GGUF"], "1\n"
        )

        assert "downloaded successfully!" in stdout
        assert return_code == 0

    def test_model_pull_hugging_face_not_gguf_should_failed_gracefully(self):
        """
        When pull a model which is not GGUF, we stop and show a message to user
        """

        stdout, stderr, return_code = popen(["pull", "BAAI/bge-reranker-v2-m3"], "")
        assert (
            "Not a GGUF model. Currently, only GGUF single file is supported." in stdout
        )
        assert return_code == 0
