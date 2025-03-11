import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiCreateMessage:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_create_message_successfully(self):
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
        log_response(json_data, "test_api_create_message_successfully")
        assert_equal(response.status_code,200)
        
        thread_id = json_data["id"]
        
        post_message_data = {
            "role": "user",
            "content": "Hello, world!"
        }
        
        post_message_url = f"http://localhost:3928/v1/threads/{thread_id}/messages"
        new_message_response = requests.post(post_message_url, json=post_message_data)
        json_data_new_message = new_message_response.json()
        log_response(json_data_new_message, "test_api_create_message_successfully")
        assert_equal(new_message_response.status_code,200)
        
        schema = {
            "type": "object",
            "properties": {
            "id": {
                "type": "string",
                "description": "Unique identifier for the message"
            },
            "object": {
                "type": "string",
                "description": "Type of object, always 'thread.message'"
            },
            "created_at": {
                "type": "integer",
                "description": "Unix timestamp of when the message was created"
            },
            "completed_at": {
                "type": "integer",
                "description": "Unix timestamp of when the message was completed"
            },
            "thread_id": {
                "type": "string",
                "description": "ID of the thread this message belongs to"
            },
            "role": {
                "type": "string",
                "description": "Role of the message sender",
                "enum": [
                "user",
                "assistant"
                ]
            },
            "status": {
                "type": "string",
                "description": "Status of the message",
                "enum": [
                "completed"
                ]
            },
            "content": {
                "type": "array",
                "items": {
                "type": "object",
                "properties": {
                    "type": {
                    "type": "string",
                    "description": "Type of content",
                    "enum": [
                        "text"
                    ]
                    },
                    "text": {
                    "type": "object",
                    "properties": {
                        "value": {
                        "type": "string",
                        "description": "The message text"
                        },
                        "annotations": {
                        "type": "array",
                        "description": "Array of annotations for the text"
                        }
                    }
                    }
                }
                }
            },
            "metadata": {
                "type": "object",
                "description": "Additional metadata for the message"
            }
            },
            "required": [
            "id",
            "object",
            "created_at",
            "completed_at",
            "thread_id",
            "role",
            "status",
            "content"
            ]
        }

        # Validate response schema
        jsonschema.validate(instance=json_data_new_message, schema=schema)
        
        assert_equal(json_data_new_message["content"][0]['text']["value"], "Hello, world!")