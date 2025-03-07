import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiGetAssistant:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_get_assistant_successfully(self):
        headers = {
            "Content-Type": "application/json",
        }
        
        data = {
            "description": "",
            "instructions": "",
            "metadata": {
                "ANY_ADDITIONAL_PROPERTY": "anything"
            },
            "model": "tinyllama:1b",
            "name": "test_assistant",
            "response_format": "auto",
            "temperature": 1,
            "tool_resources": {
                "code_interpreter": {},
                "file_search": {}
            },
            "tools": [
                {
                "type": "code_interpreter"
                }
            ],
            "top_p": 1
        }
        
        assistant_url = "http://localhost:3928/v1/assistants"
        response = requests.post(
            assistant_url, headers=headers, json=data
        )
        json_data = response.json()
        log_response(json_data, "test_api_get_assistant_successfully")
        assert_equal(response.status_code,200)
        
        assistant_id=response["id"]
        
        # Get list assistant
        assistantid_url=f"http://localhost:3928/v1/assistants/{assistant_id}"
        response_assistant = requests.get(
            assistantid_url
        )
        json_data_assistant = response_assistant.json()
        log_response(json_data_assistant, "test_api_get_assistant_successfully")
        assert_equal(response_assistant.status_code,200)
        
        schema = {
            "properties": {
            "created_at": {
                "description": "Unix timestamp (in seconds) of when the assistant was created.",
                "type": "integer"
            },
            "id": {
                "description": "The unique identifier of the assistant.",
                "type": "string"
            },
            "metadata": {
                "additionalProperties": True,
                "description": "Set of key-value pairs attached to the assistant.",
                "type": "object"
            },
            "model": {
                "description": "The model identifier used by the assistant.",
                "type": "string"
            },
            "object": {
                "description": "The object type, which is always 'assistant'.",
                "enum": [
                "assistant"
                ],
                "type": "string"
            }
            },
            "required": [
            "id",
            "object",
            "created_at",
            "model",
            "metadata"
            ],
            "type": "object"
        }

        # Validate response schema
        jsonschema.validate(instance=response_assistant, schema=schema)
        assert_equal(json_data_assistant["id"], assistant_id)
    