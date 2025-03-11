import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiGetListThread:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_get_list_thread_successfully(self):
        title = "New Thread"
        
        data = {
            "metadata": {
                "title": title
            }
        }
        
        thread_url = f"http://localhost:3928/v1/threads"
        response = requests.post(
            thread_url, json=data
        )
        json_data = response.json()
        log_response(json_data, "test_api_get_list_thread_successfully")
        assert_equal(response.status_code,200)
        
        list_thread_response = requests.get(thread_url)
        json_data_list_thread = list_thread_response.json()
        log_response(json_data_list_thread, "test_api_get_list_thread_successfully")
        assert_equal(list_thread_response.status_code,200)
        
        schema = {
            "type": "object",
            "properties": {
            "object": {
                "type": "string",
                "description": "Type of the list response, always 'list'"
            },
            "data": {
                "type": "array",
                "description": "Array of thread objects",
                "items": {
                "type": "object",
                "properties": {
                    "created_at": {
                    "type": "integer",
                    "description": "Unix timestamp of when the thread was created"
                    },
                    "id": {
                    "type": "string",
                    "description": "Unique identifier for the thread"
                    },
                    "metadata": {
                    "type": "object",
                    "properties": {
                        "title": {
                        "type": "string",
                        "description": "Title of the thread"
                        },
                        "lastMessage": {
                        "type": "string",
                        "description": "Content of the last message in the thread"
                        }
                    },
                    "description": "Metadata associated with the thread"
                    },
                    "object": {
                    "type": "string",
                    "description": "Type of object, always 'thread'"
                    }
                },
                "required": [
                    "created_at",
                    "id",
                    "object"
                ]
                }
            }
            },
            "required": [
            "object",
            "data"
            ]
        }

        # Validate response schema
        jsonschema.validate(instance=json_data_list_thread, schema=schema)
        assert_equal(json_data_list_thread["data"][0]["metadata"]['title'], title)