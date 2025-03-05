import pytest
import requests
from utils.test_runner import start_server, stop_server
import os
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal
import fnmatch


class TestApiDeleteFile:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()
        
    def test_api_del_file_successfully(self):
        # Define file path
        file_path = os.path.join("e2e-test", "api", "files", "blank.txt")

        # Upload file first
        files = {
            "file": ("blank.txt", open(file_path, "rb"), "text/plain")
        }
        data = {
            "purpose": "assistants"
        }

        file_url = "http://127.0.0.1:3928/v1/files"
        response = requests.post(file_url, files=files, data=data)

        json_data = response.json()
        log_response(json_data, "test_api_del_file_successfully")
        assert_equal(response.status_code, 200)
        
        file_id=json_data["id"]

        # Delete message with id
        file_id_url = f"http://127.0.0.1:3928/v1/files/{file_id}"
        file_response = requests.delete(file_id_url)
        json_data_file = file_response.json()
        log_response(json_data_file, "test_api_del_file_successfully")
        assert_equal(file_response.status_code,200)


        # Schema to validate
        schema = {
            "properties": {
            "deleted": {
                "description": "Indicates if the file was successfully deleted",
                "type": "boolean"
            },
            "id": {
                "description": "The ID of the deleted file",
                "type": "string"
            },
            "object": {
                "description": "Type of object, always 'file'",
                "type": "string"
            }
            },
            "required": [
            "deleted",
            "id",
            "object"
            ],
            "type": "object"
        }

        # Validate response schema
        jsonschema.validate(instance=json_data_file, schema=schema)

        # Assert content
        assert_equal(json_data_file["deleted"], True)
        assert_equal(json_data_file["id"], file_id)
        assert_equal(json_data_file["object"], "file")