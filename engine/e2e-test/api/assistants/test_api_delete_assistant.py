import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiDeleteAssistant:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_delete_assistant_successfully(self):
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
        log_response(json_data, "test_api_delete_assistant_successfully")
        assert_equal(response.status_code,200)
        
        assistant_id=json_data["id"]
        
        # Get list assistant
        assistantid_url=f"http://localhost:3928/v1/assistants/{assistant_id}"
        response_del_assistant = requests.delete(
            assistantid_url
        )
        json_data_del_assistant = response_del_assistant.json()
        log_response(json_data_del_assistant, "test_api_delete_assistant_successfully")
        assert_equal(response_del_assistant.status_code,200)
        
        schema = {
            "$schema": "http://json-schema.org/draft-07/schema#",
            "type": "object",
            "properties": {
                "deleted": {
                    "type": "boolean"
                },
                "id": {
                    "type": "string"
                },
                "object": {
                    "type": "string",
                    "enum": ["assistant.deleted"]
                }
            },
            "required": ["deleted", "id", "object"]
        }


        # Validate response schema
        jsonschema.validate(instance=json_data_del_assistant, schema=schema)
        
        # Assert content
        assert_equal(json_data_del_assistant["deleted"], True)
        assert_equal(json_data_del_assistant["id"], assistant_id)
        assert_equal(json_data_del_assistant["object"], "assistant.deleted")
    