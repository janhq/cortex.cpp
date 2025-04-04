import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from tenacity import retry, wait_exponential, stop_after_attempt
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiDefaultEngine:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_get_default_engine_successfully(self):
        # Data test
        engine= "llama-cpp"
        name= "linux-avx-x64"
        version= "b4932"
    
        data = {"version": version, "variant": name}
        post_install_url = f"http://localhost:3928/v1/engines/{engine}/install"
        response = requests.post(
            post_install_url, json=data
        )
        assert_equal(response.status_code,200)
        log_response(response.json(), "test_api_get_default_engine_successfully")
        
        get_list_url = f"http://localhost:3928/v1/engines/{engine}"
        get_default_url = f"http://localhost:3928/v1/engines/{engine}/default"

        @retry(
            wait=wait_exponential(multiplier=2, min=2, max=30), 
            stop=stop_after_attempt(5) 
        )
        def get_request(url):
            response = requests.get(url)
            assert len(response.json()) > 0

        get_request(get_list_url)
        
        response_default_engine = requests.get(get_default_url)
        json_data = response_default_engine.json()

        log_response(json_data, "test_api_get_default_engine_successfully")
        assert_equal(response_default_engine.status_code, 200)

        schema = {
            "type": "object",
            "properties": {
                "engine": {"type": "string"},
                "variant": {"type": "string"},
                "version": {"type": "string"}
            },
            "required": ["engine", "variant", "version"]
        }

        # Validate response schema
        jsonschema.validate(instance=json_data, schema=schema)
        
    def test_api_get_default_engine_failed_invalid_engine(self):
        # Data test
        engine= "invalid"
    
        get_default_url = f"http://localhost:3928/v1/engines/{engine}/default"

        response_default_engine = requests.get(get_default_url)
        json_data_get_default = response_default_engine.json()

        log_response(json_data_get_default, "test_api_get_default_engine_failed_invalid_engine")
        assert_equal(response_default_engine.status_code, 400)

        assert_equal(json_data_get_default["message"], f"Engine {engine} is not supported yet!")