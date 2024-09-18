from test_runner import run


class TestCliModelPullDirectUrl:

    def test_model_pull_with_direct_url_should_be_success(self):
        exit_code, output, error = run(
            "Pull model",
            [
                "pull",
                "https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v0.3-GGUF/blob/main/tinyllama-1.1b-chat-v0.3.Q2_K.gguf",
            ],
            timeout=None,
        )
        assert exit_code == 0, f"Model pull failed with error: {error}"
        # TODO: verify that the model has been pull successfully
        # TODO: skip this test. since download model is taking too long

