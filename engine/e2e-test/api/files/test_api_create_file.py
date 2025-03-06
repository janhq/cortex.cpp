import pytest
import requests
from utils.test_runner import start_server, stop_server
import os
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal
import fnmatch


class TestApiCreateFile:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()
        
    def test_api_create_file_successfully(self):
        # Define file path
        file_path_rel = os.path.join("e2e-test", "api", "files", "blank.txt")
        file_path = os.path.join(os.getcwd(), file_path_rel)

        log_response(file_path, "test_api_create_file_successfully")


        # # Prepare request data
        # files = {
        #     "file": ("blank.txt", open(file_path, "rb"), "text/plain")
        # }
        # data = {
        #     "purpose": "assistants"
        # }

        post_file_url = "http://127.0.0.1:3928/v1/files"
        # response = requests.post(post_file_url, files=files, data=data)
        with open(file_path, "rb") as file:
            files = {"file": ("blank.txt", file, "text/plain")}
            data = {"purpose": "assistants"}
            response = requests.post(post_file_url, files=files, data=data)
            log_response(response.text, "test_api_create_file_successfully")
            log_response(response.status_code, "test_api_create_file_successfully")

        json_data = response.json()
        log_response(json_data, "test_api_create_file_successfully")
        assert_equal(response.status_code, 200)

        # Schema to validate
        schema = {
            "type": "object",
            "properties": {
                "bytes": {"type": "integer"},
                "created_at": {"type": "integer"},
                "filename": {"type": "string"},
                "id": {"type": "string"},
                "object": {"type": "string"},
                "purpose": {"type": "string"}
            },
            "required": ["bytes", "created_at", "filename", "id", "object", "purpose"]
        }

        # Validate response schema
        jsonschema.validate(instance=json_data, schema=schema)

        # Assert content
        assert (fnmatch.fnmatch(json_data["filename"], "blank_*.txt") or json_data["filename"] == "blank.txt"), f"Filename {json_data['filename']} does not match pattern blank_*.txt or blank.txt"
        assert_equal(json_data["purpose"], "assistants")