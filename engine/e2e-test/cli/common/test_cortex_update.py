import os
import tempfile

import pytest
from utils.test_runner import run


class TestCortexUpdate:
    # We don't have stable release yet, so mark this test as skip for now
    # Only able to test with beta and nightly
    @pytest.mark.skip(reason="Stable release is not available yet")
    def test_cortex_update(self):
        exit_code, output, error = run("Update cortex", ["update"])
        assert exit_code == 0, "Something went wrong"
        assert "Updated cortex sucessfully" in output
        assert os.path.exists(os.path.join(tempfile.gettempdir()), "cortex") == False
