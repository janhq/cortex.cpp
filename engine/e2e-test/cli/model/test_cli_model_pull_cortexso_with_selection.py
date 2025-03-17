from utils.test_runner import popen
import os
from pathlib import Path


class TestCliModelPullCortexsoWithSelection:

    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()
        
    def test_pull_model_from_cortexso_should_display_list_and_allow_user_to_choose(
        self,
    ):
        stdout, stderr, return_code = popen(["pull", "tinyllama"], "1\n")

        assert "Model tinyllama downloaded successfully!" in stdout
        assert return_code == 0
