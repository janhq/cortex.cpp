import pytest
import requests
from utils.test_runner import start_server, stop_server
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal


class TestApiDeleteMessage:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_api_delete_message_successfully(self):
        title = "New Thread"
        
        data = {
            "metadata": {
                "title": title
            }
        }
        # Create new thread
        post_thread_url = f"http://localhost:3928/v1/threads"
        response = requests.post(
            post_thread_url, json=data
        )
        json_data = response.json()
        log_response(json_data, "test_api_delete_message_successfully")
        assert_equal(response.status_code,200)
        
        thread_id = json_data["id"]
        
        post_message_data = {
            "role": "user",
            "content": "Hello, world!"
        }
        
        # Create new message
        message_url = f"http://localhost:3928/v1/threads/{thread_id}/messages"
        new_message_response = requests.post(message_url, json=post_message_data)
        json_data_new_message = new_message_response.json()
        log_response(json_data_new_message, "test_api_delete_message_successfully")
        assert_equal(new_message_response.status_code,200)
        
        message_id = json_data_new_message["id"]
        
        # Delete message with id
        del_message_url = f"http://localhost:3928/v1/threads/{thread_id}/messages/{message_id}"
        del_message_response = requests.delete(del_message_url)
        json_data_del_message = del_message_response.json()
        log_response(json_data_del_message, "test_api_delete_message_successfully")
        assert_equal(del_message_response.status_code,200)
        
        schema = {
            "type": "object",
            "properties": {
            "deleted": {
                "type": "boolean",
                "description": "Indicates if the message was successfully deleted"
            },
            "id": {
                "type": "string",
                "description": "ID of the deleted message"
            },
            "object": {
                "type": "string",
                "description": "Type of object, always 'thread.message.deleted'"
            }
            },
            "required": [
            "deleted",
            "id",
            "object"
            ]
        }

        # Validate response schema
        jsonschema.validate(instance=json_data_del_message, schema=schema)
        
        assert_equal(json_data_del_message["deleted"], True)
        assert_equal(json_data_del_message["id"], message_id)
        assert_equal(json_data_del_message["object"], "thread.message.deleted")