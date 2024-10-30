import pytest
from test_runner import start_server, stop_server

import requests
import os

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

def get_repo_branches(repo_id, token):
    # API endpoint to get list of branches for each repo
    url = f"https://huggingface.co/api/models/{repo_id}/refs"
    headers = {"Authorization": f"Bearer {token}"}
    response = requests.get(url, headers=headers)

    # Check response and get the gguf branch
    if response.status_code == 200:
        branches = response.json()["branches"]
        return [branch['name'] for branch in branches if branch['name'] == 'gguf']
    else:
        print(f"Error fetching branches for {repo_id}:", response.status_code, response.json())
        return []

def get_all_repos_and_default_branches_gguf(collection_id, token):
    # Get list of repos from the collection
    repos = get_repos_in_collection(collection_id, token)
    combined_list = []

    # Iterate over each repo and fetch branches
    for repo_id in repos:
        branches = get_repo_branches(repo_id, token)
        for branch in branches:
            combined_list.append(f"{repo_id.split('/')[1]}:{branch}")

    return combined_list

collection_id = "cortexso/local-models-6683a6e29e8f3018845b16db"
token = os.getenv("HF_TOKEN")
if not token:
    raise ValueError("HF_TOKEN environment variable not set")

# Call the function and print the results
repo_branches = get_all_repos_and_default_branches_gguf(collection_id, token)

class TestCortexsoModels:

    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")
        yield
        # Teardown
        stop_server()

