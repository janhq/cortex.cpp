import pytest
from test_runner import popen, run


class TestCliModelDelete:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        # Pull model

        # TODO: using pull with branch for easy testing tinyllama:gguf for example
        popen(["pull", "tinyllama"], "1\n")

        yield

        # Teardown
        # Clean up
        run("Delete model", ["models", "delete", "tinyllama"])

    def test_models_delete_should_be_successful(self):
        exit_code, output, error = run(
            "Delete model", ["models", "delete", "tinyllama:gguf"]
        )
        assert "Model tinyllama:gguf deleted successfully" in output
        assert exit_code == 0, f"Model does not exist: {error}"
