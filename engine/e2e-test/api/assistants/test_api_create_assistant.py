import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiCreateAssistant:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_create_assistant_successfully(self):
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
        
        post_assistant_url = "http://localhost:3928/v1/assistants"
        res=requests.get(post_assistant_url)
        log_response(res.text, "test_api_create_assistant_successfully")
        
        
        response = requests.post(
            post_assistant_url, headers=headers, json=data
        )
        log_response(response.text, "test_api_create_assistant_successfully")
        json_data = response.json()
        assert_equal(response.status_code,200)
        
        schema = {
            "properties": {
            "created_at": {
                "description": "Unix timestamp (in seconds) of when the assistant was created.",
                "type": "integer"
            },
            "description": {
                "description": "The description of the assistant.",
                "type": "string"
            },
            "id": {
                "description": "The unique identifier of the assistant.",
                "type": "string"
            },
            "instructions": {
                "description": "Instructions for the assistant's behavior.",
                "type": "string"
            },
            "metadata": {
                "additionalProperties": True,
                "description": "Set of key-value pairs that can be attached to the assistant.",
                "type": "object"
            },
            "model": {
                "description": "The model identifier used by the assistant.",
                "type": "string"
            },
            "name": {
                "description": "The name of the assistant.",
                "type": "string"
            },
            "object": {
                "description": "The object type, which is always 'assistant'.",
                "enum": [
                "assistant"
                ],
                "type": "string"
            },
            "response_format": {
                "oneOf": [
                {
                    "enum": [
                    "auto"
                    ],
                    "type": "string"
                },
                {
                    "type": "object"
                }
                ]
            },
            "temperature": {
                "description": "Temperature parameter for response generation.",
                "format": "float",
                "type": "number"
            },
            "tool_resources": {
                "description": "Resources used by the assistant's tools.",
                "properties": {
                "code_interpreter": {
                    "type": "object"
                },
                "file_search": {
                    "type": "object"
                }
                },
                "type": "object"
            },
            "tools": {
                "description": "A list of tools enabled on the assistant.",
                "items": {
                "properties": {
                    "type": {
                    "enum": [
                        "code_interpreter",
                        "file_search",
                        "function"
                    ],
                    "type": "string"
                    }
                },
                "type": "object"
                },
                "type": "array"
            },
            "top_p": {
                "description": "Top p parameter for response generation.",
                "format": "float",
                "type": "number"
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
        jsonschema.validate(instance=json_data, schema=schema)
    