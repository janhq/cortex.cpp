import pytest
import requests
import time
from test_runner import (
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)
import json
import jsonschema

# logging.basicConfig(level=logging.INFO, force=True)  # Ensure logs show
# logger = logging.getLogger(__name__)

class TestApiEngine:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()
    
    
        
    # # engines install
    # def test_engines_install_llamacpp_specific_version_and_variant(self):
    #     data = {"version": "v0.1.35-27.10.24", "variant": "linux-amd64-avx-cuda-11-7"}
    #     response = requests.post(
    #         "http://localhost:3928/v1/engines/llama-cpp/install", json=data
    #     )
    #     assert response.status_code == 200
    #     with open("response-install.json", "w") as file:
    #         json.dump(response.json(), file, indent=4)

    # engines get
    def test_engines_get_llamacpp_should_be_successful(self):
        engine= "llama-cpp"
        name= "linux-amd64-avx-cuda-11-7"
        version= "v0.1.35-27.10.24"
    
        # data = {"version": version, "variant": name}
        data = {"version": "v0.1.35-27.10.24", "variant": "linux-amd64-avx-cuda-11-7"}
        post_url = f"http://localhost:3928/v1/engines/{engine}/install"
        response = requests.post(
            post_url, json=data
        )
        assert response.status_code == 200
        
        get_url = f"http://localhost:3928/v1/engines/{engine}"
        response = requests.get(get_url)
        count = 0
        while len(response.json()) == 0:
            time.sleep(1)
            response = requests.get(get_url)
            count += 1

        json_data = response.json()
        with open("e2e-test/response.json", "w") as file:
            json.dump(json_data, file, indent=4)
            # file.write(f"\nCount: {count}\n") 
        assert response.status_code == 200

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

        assert json_data[0]["engine"] == engine
        assert json_data[0]["version"] == version
        assert json_data[0]["name"] == name

        delete_url = f"http://localhost:3928/v1/engines/{engine}/install"
        delete_response = requests.delete(
            delete_url, json=data
        )
        with open("e2e-test/response_engine_uninstall.json", "w") as file:
            json.dump(delete_response.json(), file, indent=4)
        assert delete_response.status_code ==200
        assert delete_response.json()["message"] == "Engine llama-cpp uninstalled successfully!"

        get_url = f"http://localhost:3928/v1/engines/{engine}"
        get_response = requests.get(get_url)
        assert len(get_response.json()) == 0


    def test_engines_get_llamacpp_release_list(self):
        engine= "llama-cpp"

        get_url = f"http://localhost:3928/v1/engines/{engine}/releases"
        response = requests.get(get_url)
        assert response.status_code == 200

        json_data = response.json()
        with open("e2e-test/response_engine_release.json", "w") as file:
            json.dump(json_data, file, indent=4)

        schema = {
            "type": "array",
            "items": {
                "type": "object",
                "properties": {
                    "draft": {"type": "boolean"},
                    "name": {"type": "string"},
                    "prerelease": {"type": "boolean"},
                    "published_at": {"type": "string", "format": "date-time"},
                    "url": {"type": "string", "format": "uri"}
                },
                "required": ["draft", "name", "prerelease", "published_at", "url"]
            }
        }

        jsonschema.validate(instance=json_data, schema=schema)

    # def test_engines_install_llamacpp_specific_version_and_null_variant(self):
    #     data = {"version": "v0.1.35-27.10.24"}
    #     response = requests.post(
    #         "http://localhost:3928/v1/engines/llama-cpp/install", json=data
    #     )
    #     assert response.status_code == 200
    
    # # engines uninstall
    # @pytest.mark.asyncio
    # async def test_engines_install_uninstall_llamacpp_should_be_successful(self):
    #     response = requests.post("http://localhost:3928/v1/engines/llama-cpp/install")
    #     assert response.status_code == 200
    #     await wait_for_websocket_download_success_event(timeout=None)
    #     time.sleep(30)

    #     response = requests.delete("http://localhost:3928/v1/engines/llama-cpp/install")
    #     assert response.status_code == 200

    # @pytest.mark.asyncio
    # async def test_engines_install_uninstall_llamacpp_with_only_version_should_be_failed(self):
    #     # install first
    #     data = {"variant": "mac-arm64"}
    #     install_response = requests.post(
    #         "http://127.0.0.1:3928/v1/engines/llama-cpp/install", json=data
    #     )
    #     await wait_for_websocket_download_success_event(timeout=120)
    #     assert install_response.status_code == 200

    #     data = {"version": "v0.1.35"}
    #     response = requests.delete(
    #         "http://localhost:3928/v1/engines/llama-cpp/install", json=data
    #     )
    #     assert response.status_code == 400
    #     assert response.json()["message"] == "No variant provided"

    # @pytest.mark.asyncio
    # async def test_engines_install_uninstall_llamacpp_with_variant_should_be_successful(self):
    #     # install first
    #     data = {"variant": "mac-arm64"}
    #     install_response = requests.post(
    #         "http://127.0.0.1:3928/v1/engines/llama-cpp/install", json=data
    #     )
    #     await wait_for_websocket_download_success_event(timeout=120)
    #     assert install_response.status_code == 200

    #     response = requests.delete("http://127.0.0.1:3928/v1/engines/llama-cpp/install")
    #     assert response.status_code == 200

    # def test_engines_install_uninstall_llamacpp_with_specific_variant_and_version_should_be_successful(
    #     self,
    # ):
    #     data = {"variant": "mac-arm64", "version": "v0.1.35"}
    #     # install first
    #     install_response = requests.post(
    #         "http://localhost:3928/v1/engines/llama-cpp/install", json=data
    #     )
    #     assert install_response.status_code == 200

    #     response = requests.delete(
    #         "http://localhost:3928/v1/engines/llama-cpp/install", json=data
    #     )
    #     assert response.status_code == 200

    