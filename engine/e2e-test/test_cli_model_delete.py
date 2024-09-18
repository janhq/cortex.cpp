import pytest
from test_runner import run


class TestCliModelDelete:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        # Pull model
        run("Pull Model", ["pull", "tinyllama"], 120)

        yield

        # Teardown
        # Clean up
        run("Delete model", ["models", "delete", "tinyllama"])

    def test_models_delete_should_be_successful(self):
        exit_code, output, error = run(
            "Delete model", ["models", "delete", "tinyllama"]
        )
        assert "The model tinyllama was deleted" in output
        assert exit_code == 0, f"Model does not exist: {error}"
