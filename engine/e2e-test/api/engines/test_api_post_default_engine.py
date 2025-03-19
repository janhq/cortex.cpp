import pytest
import requests
from utils.test_runner import start_server, stop_server
from tenacity import retry, wait_exponential, stop_after_attempt
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiSetDefaultEngine:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_set_default_engine_successfully(self):
        # Data test
        engine= "llama-cpp"
        name= "linux-avx-x64"
        version= "b4920"
    
        data = {"version": version, "variant": name}
        post_install_url = f"http://localhost:3928/v1/engines/{engine}/install"
        response = requests.post(
            post_install_url, json=data
        )
        assert_equal(response.status_code,200)
        log_response(response.json(), "test_api_get_default_engine_successfully")
        
        get_list_url = f"http://localhost:3928/v1/engines/{engine}"
        post_default_url = f"http://localhost:3928/v1/engines/{engine}/default"

        @retry(
            wait=wait_exponential(multiplier=2, min=2, max=30), 
            stop=stop_after_attempt(5) 
        )
        def get_request(url):
            response = requests.get(url)
            assert len(response.json()) > 0

        get_request(get_list_url)
        
        response_set_default_engine = requests.post(post_default_url, json=data)
        json_data = response_set_default_engine.json()

        log_response(json_data, "test_api_set_default_engine_successfully")
        assert_equal(response_set_default_engine.status_code, 200)

        assert_equal(json_data["message"], f"Engine {name} {version.lstrip('v')} set as default")