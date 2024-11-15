import pytest
import requests
from test_runner import (
    run,
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)


class TestApiModelPullDirectUrl:
    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        stop_server()
        success = start_server()
        if not success:
            raise Exception("Failed to start server")
        # Delete model if exists
        run(
            "Delete model",
            [
                "models",
                "delete",
                "afrideva:zephyr-smol_llama-100m-sft-full-GGUF:zephyr-smol_llama-100m-sft-full.q2_k.gguf",
            ],
        )
        yield

        # Teardown
        run(
            "Delete model",
            [
                "models",
                "delete",
                "afrideva:zephyr-smol_llama-100m-sft-full-GGUF:zephyr-smol_llama-100m-sft-full.q2_k.gguf",
            ],
        )
        stop_server()

    @pytest.mark.asyncio
    async def test_model_pull_with_direct_url_should_be_success(self):
        myobj = {
            "model": "https://huggingface.co/afrideva/zephyr-smol_llama-100m-sft-full-GGUF/blob/main/zephyr-smol_llama-100m-sft-full.q2_k.gguf"
        }
        response = requests.post("http://localhost:3928/v1/models/pull", json=myobj)
        assert response.status_code == 200
        await wait_for_websocket_download_success_event(timeout=None)
        get_model_response = requests.get(
            "http://127.0.0.1:3928/v1/models/afrideva:zephyr-smol_llama-100m-sft-full-GGUF:zephyr-smol_llama-100m-sft-full.q2_k.gguf"
        )
        assert get_model_response.status_code == 200
        assert (
            get_model_response.json()["model"]
            == "afrideva:zephyr-smol_llama-100m-sft-full-GGUF:zephyr-smol_llama-100m-sft-full.q2_k.gguf"
        )

    @pytest.mark.asyncio
    async def test_model_pull_with_direct_url_should_have_desired_name(self):
        myobj = {
            "model": "https://huggingface.co/afrideva/zephyr-smol_llama-100m-sft-full-GGUF/blob/main/zephyr-smol_llama-100m-sft-full.q2_k.gguf",
            "name": "smol_llama_100m"
        }
        response = requests.post("http://localhost:3928/v1/models/pull", json=myobj)
        assert response.status_code == 200
        await wait_for_websocket_download_success_event(timeout=None)
        get_model_response = requests.get(
            "http://127.0.0.1:3928/v1/models/afrideva:zephyr-smol_llama-100m-sft-full-GGUF:zephyr-smol_llama-100m-sft-full.q2_k.gguf"
        )
        assert get_model_response.status_code == 200
        print(get_model_response.json()["name"])
        assert (
            get_model_response.json()["name"]
            == "smol_llama_100m"
        )
