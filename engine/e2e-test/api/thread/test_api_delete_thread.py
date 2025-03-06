import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiDeleteThread:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_delete_thread_successfully(self):
        title = "New thread"
        
        data = {
            "metadata": {
                "title": title
            }
        }
        
        thread_url = f"http://localhost:3928/v1/threads"
        response = requests.post(
            thread_url, json=data
        )
        json_data = response.json()
        log_response(json_data, "test_api_delete_thread_successfully")
        assert_equal(response.status_code,200)
        thread_id = json_data["id"]
        
        thread_id_url = f"http://localhost:3928/v1/threads/{thread_id}"
        thread_response = requests.delete(thread_id_url)
        json_data_thread = thread_response.json()
        log_response(json_data_thread, "test_api_delete_thread_successfully")
        assert_equal(thread_response.status_code,200)
        
        schema = {
            "type": "object",
            "properties": {
            "deleted": {
                "type": "boolean",
                "description": "Indicates if the thread was successfully deleted"
            },
            "id": {
                "type": "string",
                "description": "ID of the deleted thread"
            },
            "object": {
                "type": "string",
                "description": "Type of object, always 'thread.deleted'"
            }
            },
            "required": [
            "deleted",
            "id",
            "object"
            ]
        }

        # Validate response schema
        jsonschema.validate(instance=json_data_thread, schema=schema)
        
        assert_equal(json_data_thread["deleted"], True)
        assert_equal(json_data_thread["id"], thread_id)
        assert_equal(json_data_thread["object"], "thread.deleted")