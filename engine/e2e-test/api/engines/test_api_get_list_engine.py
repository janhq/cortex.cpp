import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from tenacity import retry, wait_exponential, stop_after_attempt
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiEngineList:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_get_list_engines_successfully(self):
        # Data test
        engine= "llama-cpp"
        name= "linux-amd64-avx"
        version= "v0.1.35-27.10.24"
    
        data = {"version": version, "variant": name}
        post_install_url = f"http://localhost:3928/v1/engines/{engine}/install"
        response = requests.post(
            post_install_url, json=data
        )
        assert_equal(response.status_code,200)
        
        get_list_url = f"http://localhost:3928/v1/engines/{engine}"

        @retry(
            wait=wait_exponential(multiplier=2, min=2, max=30), 
            stop=stop_after_attempt(5) 
        )
        def get_request(url):
            response = requests.get(url)
            assert len(response.json()) > 0
            return response

        response_get_list = get_request(get_list_url)
        json_data = response_get_list.json()

        log_response(json_data, "test_api_get_list_engines_successfully")
        assert_equal(response_get_list.status_code, 200)

        schema = {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "engine": {"type": "string"},
                    "name": {"type": "string"},
                    "version": {"type": "string"}
                },
                "required": ["engine", "name", "version"]
            }
        }

        # Validate response schema
        jsonschema.validate(instance=json_data, schema=schema)
        
        assert_equal(len(json_data), 1)
        assert_equal(json_data[0]["engine"], engine)
        assert_equal(json_data[0]["version"], version)
        assert_equal(json_data[0]["name"], name)