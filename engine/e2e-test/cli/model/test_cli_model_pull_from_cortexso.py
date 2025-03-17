import pytest
from utils.test_runner import run


class TestCliModelPullCortexso:

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
            ["pull", "tinyllama"],
            timeout=None,
        )
        assert exit_code == 0, f"Model pull failed with error: {error}"
        # TODO: verify that the model has been pull successfully
        # TODO: skip this test. since download model is taking too long

