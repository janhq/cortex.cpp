import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiCreateThread:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_create_thread_successfully(self):
        title = "New Thread"
        
        data = {
            "metadata": {
                "title": title
            }
        }
        
        post_thread_url = f"http://localhost:3928/v1/threads"
        response = requests.post(
            post_thread_url, json=data
        )
        json_data = response.json()
        log_response(json_data, "test_api_create_thread_successfully")
        assert_equal(response.status_code,200)
        
        schema = {
            "$schema": "http://json-schema.org/draft-07/schema#",
            "type": "object",
            "properties": {
                "created_at": {
                    "type": "integer"
                },
                "id": {
                    "type": "string"
                },
                "metadata": {
                    "type": "object",
                    "properties": {
                        "title": {
                            "type": "string"
                        }
                    },
                    "required": ["title"],
                },
                "object": {
                    "type": "string"
                }
            },
            "required": ["created_at", "id", "metadata", "object"],
        }

        # Validate response schema
        jsonschema.validate(instance=json_data, schema=schema)
        
        assert_equal(json_data["metadata"]['title'], title)