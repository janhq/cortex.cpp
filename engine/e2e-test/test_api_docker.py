import pytest
import requests
from utils.test_runner import wait_for_websocket_download_success_event

repo_branches = ["tinyllama:1b"]


class TestCortexsoModels:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        yield

    @pytest.mark.parametrize("model_url", repo_branches)
    @pytest.mark.asyncio
    async def test_models_on_cortexso_hub(self, model_url):
        print("Pull model from cortexso hub")
        # Pull model from cortexso hub
        json_body = {"model": model_url}
        response = requests.post("http://localhost:3928/v1/models/pull", json=json_body)
        assert response.status_code == 200, f"Failed to pull model: {model_url}"

        await wait_for_websocket_download_success_event(timeout=None)

        print("Check if the model was pulled successfully")
        # Check if the model was pulled successfully
        get_model_response = requests.get(
            f"http://127.0.0.1:3928/v1/models/{model_url}"
        )
        assert (
            get_model_response.status_code == 200
        ), f"Failed to fetch model: {model_url}"
        assert (
            get_model_response.json()["model"] == model_url
        ), f"Unexpected model name for: {model_url}"

        print("Check if the model is available in the list of models")
        # Check if the model is available in the list of models
        response = requests.get("http://localhost:3928/v1/models")
        assert response.status_code == 200
        models = [i["id"] for i in response.json()["data"]]
        assert model_url in models, f"Model not found in list: {model_url}"
        
        # TODO(sang) bypass for now. Re-enable when we publish new stable version for llama-cpp engine
        # print("Start the model")
        # # Start the model
        # response = requests.post(
        #     "http://localhost:3928/v1/models/start", json=json_body
        # )
        # print(response.json())
        # assert response.status_code == 200, f"status_code: {response.status_code}"

        # print("Send an inference request")
        # # Send an inference request
        # inference_json_body = {
        #     "frequency_penalty": 0.2,
        #     "max_tokens": 4096,
        #     "messages": [{"content": "", "role": "user"}],
        #     "model": model_url,
        #     "presence_penalty": 0.6,
        #     "stop": ["End"],
        #     "stream": False,
        #     "temperature": 0.8,
        #     "top_p": 0.95,
        # }
        # response = requests.post(
        #     "http://localhost:3928/v1/chat/completions",
        #     json=inference_json_body,
        #     headers={"Content-Type": "application/json"},
        # )
        # assert (
        #     response.status_code == 200
        # ), f"status_code: {response.status_code} response: {response.json()}"

        # print("Stop the model")
        # # Stop the model
        # response = requests.post("http://localhost:3928/v1/models/stop", json=json_body)
        # assert response.status_code == 200, f"status_code: {response.status_code}"
