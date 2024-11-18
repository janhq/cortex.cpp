import pytest
from test_runner import popen, run
from test_runner import start_server, stop_server

class TestCliModelDelete:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        # Pull model

        run("Pull model", ["pull", "tinyllama:gguf"], timeout=None,)

        yield

        # Teardown
        # Clean up
        run("Delete model", ["models", "delete", "tinyllama:gguf"])
        stop_server()

    def test_models_delete_should_be_successful(self):
        exit_code, output, error = run(
            "Delete model", ["models", "delete", "tinyllama:gguf"]
        )
        assert "Model tinyllama:gguf deleted successfully" in output
        assert exit_code == 0, f"Model does not exist: {error}"
