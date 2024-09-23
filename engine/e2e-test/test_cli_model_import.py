import pytest
from test_runner import run

class TestCliModelImport:

    @pytest.mark.skipif(True, reason="Expensive test. Only test when you have local gguf file.")
    def test_model_import_should_be_success(self):

        exit_code, output, error = run(
            "Pull model", ["models", "import", "--model_id","test_model","--model_path","/path/to/local/gguf"],
            timeout=None
        )
        assert exit_code == 0, f"Model import failed failed with error: {error}"
        # TODO: skip this test. since download model is taking too long