import pytest
import requests
import os
import yaml

from pathlib import Path
from utils.test_runner import (
    run,
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)

collection_id = "cortexso/local-models-6683a6e29e8f3018845b16db"
token = os.getenv("HF_TOKEN")
if not token:
    raise ValueError("HF_TOKEN environment variable not set")

def get_repos_in_collection(collection_id, token):
    # API endpoint to get list of repos in the collection
    url = f"https://huggingface.co/api/collections/{collection_id}"
    headers = {"Authorization": f"Bearer {token}"}
    response = requests.get(url, headers=headers)

    # Check response and retrieve repo IDs if successful
    if response.status_code == 200:
        return [repo['id'] for repo in response.json()["items"]]
    else:
        print("Error fetching repos:", response.status_code, response.json())
        return []

def get_repo_default_branch(repo_id, token):
    # Direct link to metadata.yaml on the main branch
    url = f"https://huggingface.co/{repo_id}/resolve/main/metadata.yml"
    headers = {"Authorization": f"Bearer {token}"}
    response = requests.get(url, headers=headers)

    # Check response and retrieve the 'default' field value
    if response.status_code == 200:
        # Read YAML content from response text
        metadata = yaml.safe_load(response.text)
        return metadata.get("default")
    else:
        print(f"Error fetching metadata for {repo_id}:", response.status_code, response.json())
        return None

def get_all_repos_and_default_branches_from_metadata(collection_id, token):
    # Get list of repos from the collection
    repos = get_repos_in_collection(collection_id, token)
    combined_list = []

    # Iterate over each repo and fetch the default branch from metadata
    for repo_id in repos:
        default_branch = get_repo_default_branch(repo_id, token)
        if default_branch and "gguf" in default_branch:
            combined_list.append(f"{repo_id.split('/')[1]}:{default_branch}")

    return combined_list

#Call the function and print the results
repo_branches = get_all_repos_and_default_branches_from_metadata(collection_id, token)

class TestCortexsoModels:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self, request):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")
        # Delete model if exists
        for model_url in repo_branches:
            run(
                "Delete model",
                [
                    "models",
                    "delete",
                    model_url,
                ],
            )
        yield

        # Teardown
        for model_url in repo_branches:
            run(
                "Delete model",
                [
                    "models",
                    "delete",
                    model_url,
                ],
            )
        stop_server()

    @pytest.mark.parametrize("model_url", repo_branches)
    @pytest.mark.asyncio
    async def test_models_on_cortexso_hub(self, model_url):

        # Pull model from cortexso hub
        json_body = {
            "model": model_url
        }
        response = requests.post("http://localhost:3928/v1/models/pull", json=json_body)
        assert response.status_code == 200, f"Failed to pull model: {model_url}"
        
        await wait_for_websocket_download_success_event(timeout=None)
        
        # Check if the model was pulled successfully
        get_model_response = requests.get(
            f"http://127.0.0.1:3928/v1/models/{model_url}"
        )
        assert get_model_response.status_code == 200, f"Failed to fetch model: {model_url}"
        assert (
            get_model_response.json()["model"] == model_url
        ), f"Unexpected model name for: {model_url}"

        # Check if the model is available in the list of models
        response = requests.get("http://localhost:3928/v1/models")
        assert response.status_code == 200
        models = [i["id"] for i in response.json()["data"]]
        assert model_url in models, f"Model not found in list: {model_url}"

        # Install Engine
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "llama-cpp"], timeout=None, capture = False
        )
        root = Path.home()
        assert os.path.exists(root / "cortexcpp" / "engines" / "cortex.llamacpp" / "version.txt")
        assert exit_code == 0, f"Install engine failed with error: {error}"

        # Start the model
        response = requests.post("http://localhost:3928/v1/models/start", json=json_body)
        assert response.status_code == 200, f"status_code: {response.status_code}"

        # Send an inference request
        inference_json_body = {
            "frequency_penalty": 0.2,
            "max_tokens": 4096,
            "messages": [
                {
                "content": "",
                "role": "user"
                }
            ],
            "model": model_url,
            "presence_penalty": 0.6,
            "stop": [
                "End"
            ],
            "stream": False,
            "temperature": 0.8,
            "top_p": 0.95
            }
        response = requests.post("http://localhost:3928/v1/chat/completions", json=inference_json_body, headers={"Content-Type": "application/json"})
        assert response.status_code == 200, f"status_code: {response.status_code} response: {response.json()}"

        # Stop the model
        response = requests.post("http://localhost:3928/v1/models/stop", json=json_body)
        assert response.status_code == 200, f"status_code: {response.status_code}"

        # Uninstall Engine
        exit_code, output, error = run(
            "Uninstall engine", ["engines", "uninstall", "llama-cpp"]
        )
        assert "Engine llama-cpp uninstalled successfully!" in output
        assert exit_code == 0, f"Install engine failed with error: {error}"
