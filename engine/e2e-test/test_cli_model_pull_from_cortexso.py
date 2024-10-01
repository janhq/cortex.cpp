import pytest
from test_runner import run


class TestCliModelPullCortexso:

    def test_model_pull_with_direct_url_should_be_success(self):
        exit_code, output, error = run(
            "Pull model",
            ["pull", "tinyllama"],
            timeout=None,
        )
        assert exit_code == 0, f"Model pull failed with error: {error}"
        # TODO: verify that the model has been pull successfully
        # TODO: skip this test. since download model is taking too long

