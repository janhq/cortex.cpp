import pytest
import requests
import time
from utils.test_runner import (
    run,
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)

class TestApiModel:
    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup        
        success = start_server()
        if not success:
            raise Exception("Failed to start server")
        # Delete model if exists
        yield

        # Teardown
        stop_server()
        
    # Pull with direct url    
    @pytest.mark.asyncio
    async def test_model_pull_with_direct_url_should_be_success(self):
        run(
            "Delete model",
            [
                "models",
                "delete",
                "afrideva:zephyr-smol_llama-100m-sft-full-GGUF:zephyr-smol_llama-100m-sft-full.q2_k.gguf",
            ],
        )
        
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
        
        run(
            "Delete model",
            [
                "models",
                "delete",
                "afrideva:zephyr-smol_llama-100m-sft-full-GGUF:zephyr-smol_llama-100m-sft-full.q2_k.gguf",
            ],
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
        
        run(
            "Delete model",
            [
                "models",
                "delete",
                "afrideva:zephyr-smol_llama-100m-sft-full-GGUF:zephyr-smol_llama-100m-sft-full.q2_k.gguf",
            ],
        )
        
    @pytest.mark.asyncio
    async def test_models_start_stop_should_be_successful(self):
        print("Install engine")
        response = requests.post("http://localhost:3928/v1/engines/llama-cpp/install")
        assert response.status_code == 200
        await wait_for_websocket_download_success_event(timeout=None)
        # TODO(sang) need to fix for cuda download
        time.sleep(30)

        print("Pull model")
        json_body = {"model": "tinyllama:gguf"}
        response = requests.post("http://localhost:3928/v1/models/pull", json=json_body)
        assert response.status_code == 200, f"Failed to pull model: tinyllama:gguf"
        await wait_for_websocket_download_success_event(timeout=None)
        
        # get API
        print("Get model")
        response = requests.get("http://localhost:3928/v1/models/tinyllama:gguf")
        assert response.status_code == 200
        
        # list API
        print("List model")
        response = requests.get("http://localhost:3928/v1/models")
        assert response.status_code == 200

        print("Start model")
        json_body = {"model": "tinyllama:gguf"}
        response = requests.post(
            "http://localhost:3928/v1/models/start", json=json_body
        )
        assert response.status_code == 200, f"status_code: {response.status_code}"

        print("Stop model")
        response = requests.post("http://localhost:3928/v1/models/stop", json=json_body)
        assert response.status_code == 200, f"status_code: {response.status_code}"
                
        # update API
        print("Update model")
        body_json = {'model': 'tinyllama:gguf'}
        response = requests.patch("http://localhost:3928/v1/models/tinyllama:gguf", json = body_json)        
        assert response.status_code == 200

        # delete API
        print("Delete model")
        response = requests.delete("http://localhost:3928/v1/models/tinyllama:gguf")
        assert response.status_code == 200
        
    def test_models_sources_api(self):
        json_body = {"source": "https://huggingface.co/cortexso/tinyllama"}
        response = requests.post(
            "http://localhost:3928/v1/models/sources", json=json_body
        )
        assert response.status_code == 200, f"status_code: {response.status_code}"
        
        json_body = {"source": "https://huggingface.co/cortexso/tinyllama"}
        response = requests.delete(
            "http://localhost:3928/v1/models/sources", json=json_body
        )
        assert response.status_code == 200, f"status_code: {response.status_code}"