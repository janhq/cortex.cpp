import pytest
import requests
from utils.test_runner import start_server, stop_server
import os
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal
import fnmatch


class TestApiGetListFile:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()
        
    def test_api_get_list_file_successfully(self):
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
        log_response(json_data, "test_api_get_list_file_successfully")
        assert_equal(response.status_code, 200)

        # Get list message
        list_file_response = requests.get(file_url)
        json_data_list_file = list_file_response.json()
        log_response(json_data_list_file, "test_api_get_list_file_successfully")
        assert_equal(list_file_response.status_code,200)


        # Schema to validate
        schema = {
            "properties": {
            "data": {
                "items": {
                "properties": {
                    "bytes": {
                    "type": "integer",
                    "examples": [
                        3211109
                    ]
                    },
                    "created_at": {
                    "type": "integer",
                    "examples": [
                        1733942093
                    ]
                    },
                    "filename": {
                    "type": "string",
                    "examples": [
                        "Enterprise_Application_Infrastructure_v2_20140903_toCTC_v1.0.pdf"
                    ]
                    },
                    "id": {
                    "type": "string",
                    "examples": [
                        "file-0001KNKPTDDAQSDVEQGRBTCTNJ"
                    ]
                    },
                    "object": {
                    "type": "string",
                    "examples": [
                        "file"
                    ]
                    },
                    "purpose": {
                    "type": "string",
                    "examples": [
                        "assistants"
                    ]
                    }
                },
                "type": "object"
                },
                "type": "array"
            },
            "object": {
                "type": "string",
                "examples": [
                "list"
                ]
            }
            },
            "type": "object"
        }

        # Validate response schema
        jsonschema.validate(instance=json_data_list_file, schema=schema)

        # Assert content
        assert fnmatch.fnmatch(json_data_list_file["data"][0]["filename"], "blank_*.txt"), f"Filename {json_data["filename"]} does not match pattern blank_*.txt"