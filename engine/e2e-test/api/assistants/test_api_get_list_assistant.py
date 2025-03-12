import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiGetListAssistant:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_get_list_assistant_successfully(self):
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
        log_response(json_data, "test_api_get_list_assistant_successfully")
        assert_equal(response.status_code,200)
        
        # Get list assistant
        response_list_assistant = requests.get(
            assistant_url
        )
        json_data_list_assistant = response_list_assistant.json()
        log_response(json_data_list_assistant, "test_api_get_list_assistant_successfully")
        assert_equal(response_list_assistant.status_code,200)
        
        schema = {
            "$schema": "http://json-schema.org/draft-07/schema#",
            "type": "object",
            "properties": {
                "object": {
                    "type": "string",
                    "enum": ["list"]
                },
                "data": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "created_at": {
                                "type": "integer"
                            },
                            "description": {
                                "type": "string"
                            },
                            "id": {
                                "type": "string"
                            },
                            "instructions": {
                                "type": "string"
                            },
                            "metadata": {
                                "type": "object",
                                "additionalProperties": {
                                    "type": "string"
                                }
                            },
                            "model": {
                                "type": "string"
                            },
                            "name": {
                                "type": "string"
                            },
                            "object": {
                                "type": "string",
                                "enum": ["assistant"]
                            },
                            "temperature": {
                                "type": "number"
                            },
                            "tool_resources": {
                                "type": "object",
                                "properties": {
                                    "file_search": {
                                        "type": "object",
                                        "properties": {
                                            "vector_store_ids": {
                                                "type": "array",
                                                "items": {
                                                    "type": "string"
                                                }
                                            }
                                        },
                                        "required": ["vector_store_ids"]
                                    }
                                },
                                "required": ["file_search"]
                            },
                            "tools": {
                                "type": "array",
                                "items": {
                                    "type": "object",
                                    "properties": {
                                        "type": {
                                            "type": "string",
                                            "enum": ["code_interpreter"]
                                        }
                                    },
                                    "required": ["type"]
                                }
                            },
                            "top_p": {
                                "type": "number"
                            }
                        },
                        "required": [
                            "created_at",
                            "description",
                            "id",
                            "instructions",
                            "metadata",
                            "model",
                            "name",
                            "object",
                            "temperature",
                            "tool_resources",
                            "tools",
                            "top_p"
                        ]
                    }
                }
            },
            "required": ["object", "data"]
        }

        # Validate response schema
        jsonschema.validate(instance=json_data_list_assistant, schema=schema)
    