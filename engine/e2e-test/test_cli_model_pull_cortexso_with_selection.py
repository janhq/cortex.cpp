from test_runner import popen


class TestCliModelPullCortexsoWithSelection:

    def test_pull_model_from_cortexso_should_display_list_and_allow_user_to_choose(
        self,
    ):
        stdout, stderr, return_code = popen(["pull", "tinyllama"], "1\n")

        assert "Model tinyllama downloaded successfully!" in stdout
        assert return_code == 0
