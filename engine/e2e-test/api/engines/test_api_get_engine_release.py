import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from tenacity import retry, wait_exponential, stop_after_attempt
from utils.logger import log_response
from utils.assertion import assert_equal, assert_contains


class TestApiEngineRelease:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_get_engine_release_successfully(self):
        # Data test
        engine= "llama-cpp"
        get_release_url = f"http://localhost:3928/v1/engines/{engine}/releases"

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

        log_response(json_data, "test_api_get_engine_release_successfully")
        assert_equal(response_engine_release.status_code, 200)

        schema = {
            "$schema": "http://json-schema.org/draft-07/schema#",
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                "draft": { "type": "boolean" },
                "name": { "type": "string" },
                "prerelease": { "type": "boolean" },
                "published_at": { "type": "string", "format": "date-time" },
                "url": { "type": "string", "format": "uri" }
                },
                "required": ["draft", "name", "prerelease", "published_at", "url"]
            }
        }

        # Validate response schema
        jsonschema.validate(instance=json_data, schema=schema)
        
    def test_api_ge_engine_release_failed_invalid_engine(self):
        # Data test
        engine= "invalid"
    
        get_default_url = f"http://localhost:3928/v1/engines/{engine}/releases"

        response_default_engine = requests.get(get_default_url)
        json_data_get_default = response_default_engine.json()

        log_response(json_data_get_default, "test_api_ge_engine_release_failed_invalid_engine")
        assert_equal(response_default_engine.status_code, 400)

        assert_contains(json_data_get_default["message"], "Not Found")