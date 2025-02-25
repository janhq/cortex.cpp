import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from tenacity import retry, wait_exponential, stop_after_attempt
from utils.logger import log_response
from utils.assertion import assert_equal, assert_contains


class TestApiEngineReleaseLatest:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_get_engine_release_latest_successfully(self):
        # Data test
        engine= "llama-cpp"
        get_release_url = f"http://localhost:3928/v1/engines/{engine}/releases/latest"

        @retry(
            wait=wait_exponential(multiplier=2, min=2, max=30), 
            stop=stop_after_attempt(5) 
        )
        def get_request(url):
            response = requests.get(url)
            assert len(response.json()) > 0

        get_request(get_release_url)
        
        response_engine_release = requests.get(get_release_url)
        json_data = response_engine_release.json()

        log_response(json_data, "test_api_get_engine_release_latest_successfully")
        assert_equal(response_engine_release.status_code, 200)

        schema = {
            "$schema": "https://json-schema.org/draft/2020-12/schema",
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                "created_at": {
                    "type": "string",
                    "format": "date-time"
                },
                "download_count": {
                    "type": "integer",
                    "minimum": 0
                },
                "name": {
                    "type": "string"
                },
                "size": {
                    "type": "integer",
                    "minimum": 0
                }
                },
                "required": ["created_at", "download_count", "name", "size"]
            }
        }


        # Validate response schema
        jsonschema.validate(instance=json_data, schema=schema)