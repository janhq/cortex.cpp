import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from tenacity import retry, wait_exponential, stop_after_attempt
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiEngineReleaseVersion:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_get_engine_release_version_successfully(self):
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
            return response

        response_engine_release = get_request(get_release_url)
        json_data_engine_release = response_engine_release.json()

        log_response(json_data_engine_release, "test_api_get_engine_release_version_successfully")
        assert_equal(response_engine_release.status_code, 200)
        
        version = json_data_engine_release[0]["name"]
        
        # send request for release with version
        
        get_release_version_url = f"http://localhost:3928/v1/engines/{engine}/releases/{version}"
        log_response(get_release_version_url, "test_api_get_engine_release_version_successfully")

        response_engine_release_version = requests.get(get_release_version_url)
        json_data_engine_release_version = response_engine_release_version.json()

        log_response(json_data_engine_release_version, "test_api_get_engine_release_version_successfully")
        assert_equal(response_engine_release_version.status_code, 200)

        schema = {
            "$schema": "http://json-schema.org/draft-07/schema#",
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
        jsonschema.validate(instance=json_data_engine_release_version, schema=schema)