import pytest
import requests
from utils.test_runner import start_server, stop_server
import os
import platform
import jsonschema
from utils.logger import log_response
from utils.assertion import assert_equal
import fnmatch
import io # For creating in-memory files
import tempfile # For temporary files on disk if needed
import json # For sending JSON data
from hypothesis import given, strategies as st, settings, HealthCheck

# --- Constants ---
BASE_URL = "http://127.0.0.1:3928/v1"
POST_FILE_URL = f"{BASE_URL}/files"
EXPECTED_PURPOSE = "assistants"
EXPECTED_OBJECT = "file"
REQUEST_TIMEOUT = 15 # Default timeout for requests in seconds

# --- Helper Function ---
def is_server_error(status_code):
    """Checks if the status code indicates a server-side error (5xx)."""
    return 500 <= status_code < 600

# --- Test Class ---
class TestApiCreateFile:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            pytest.fail("Failed to start server", pytrace=False)

        yield

        # Teardown
        stop_server()

    # ---- Success Case ----
    def test_api_create_file_successfully(self, tmp_path):
        """Verify successful file upload with valid parameters."""
        test_name = "test_api_create_file_successfully"
        file_path = tmp_path / "blank.txt"
        file_content = b"This is a test file."
        file_path.write_bytes(file_content)
        log_response(f"Test file path: {file_path}", test_name)

        with open(file_path, "rb") as file:
            files = {"file": ("blank.txt", file, "text/plain")}
            data = {"purpose": EXPECTED_PURPOSE}
            response = requests.post(POST_FILE_URL, files=files, data=data, timeout=REQUEST_TIMEOUT)

            log_response(f"Status Code: {response.status_code}", test_name)
            log_response(f"Response Body: {response.text}", test_name)

        assert_equal(response.status_code, 200)
        json_data = response.json()

        log_response(f"JSON Response: {json_data}", test_name)

        # Schema to validate
        schema = {
            "type": "object",
            "properties": {
                "bytes": {"type": "integer"},
                "created_at": {"type": "integer"},
                "filename": {"type": "string"},
                "id": {"type": "string", "pattern": "^file-"}, # IDs often have prefixes
                "object": {"type": "string", "enum": [EXPECTED_OBJECT]},
                "purpose": {"type": "string", "enum": [EXPECTED_PURPOSE]}
            },
            "required": ["bytes", "created_at", "filename", "id", "object", "purpose"]
        }

        # Validate response schema
        try:
            jsonschema.validate(instance=json_data, schema=schema)
        except jsonschema.exceptions.ValidationError as e:
             pytest.fail(f"Response schema validation failed: {e}", pytrace=False)

        # Assert content
        assert (fnmatch.fnmatch(json_data["filename"], "blank_*.txt") or json_data["filename"] == "blank.txt"), \
            f"Filename {json_data['filename']} does not match pattern blank_*.txt or blank.txt"
        assert_equal(json_data["purpose"], EXPECTED_PURPOSE)
        assert_equal(json_data["bytes"], len(file_content))
        assert json_data["id"].startswith("file-") # Example: Check ID prefix


    # ---- Tests for Missing/Invalid Parts ----

    def test_api_create_file_missing_file_part(self):
        """Verify API handles requests missing the 'file' multipart field."""
        test_name = "test_api_create_file_missing_file_part"
        data = {"purpose": EXPECTED_PURPOSE}
        # No 'files' argument passed to requests.post
        response = requests.post(POST_FILE_URL, data=data, timeout=REQUEST_TIMEOUT)

        log_response(f"Status Code: {response.status_code}", test_name)
        log_response(f"Response Body: {response.text}", test_name)

        # Expecting a client error (e.g., 400 Bad Request) because 'file' is required.
        assert 400 <= response.status_code < 500, f"Expected 4xx status code, got {response.status_code}"
        assert not is_server_error(response.status_code), "Server error occurred"

    def test_api_create_file_missing_purpose_part(self, tmp_path):
        """Verify API handles requests missing the 'purpose' multipart field."""
        test_name = "test_api_create_file_missing_purpose_part"
        file_path = tmp_path / "missing_purpose.txt"
        file_path.write_bytes(b"content")

        with open(file_path, "rb") as file:
            files = {"file": (file_path.name, file, "text/plain")}
            # No 'data' argument passed to requests.post
            response = requests.post(POST_FILE_URL, files=files, timeout=REQUEST_TIMEOUT)

            log_response(f"Status Code: {response.status_code}", test_name)
            log_response(f"Response Body: {response.text}", test_name)

        # Expecting a client error (e.g., 400 Bad Request) because 'purpose' is likely required.
        assert 400 <= response.status_code < 500, f"Expected 4xx status code, got {response.status_code}"
        assert not is_server_error(response.status_code), "Server error occurred"

    def test_api_create_file_empty_content(self):
        """Verify API handles uploading an empty file (0 bytes)."""
        test_name = "test_api_create_file_empty_content"
        file_content = b""
        file_obj = io.BytesIO(file_content)
        filename = "empty_file.txt"

        files = {"file": (filename, file_obj, "application/octet-stream")}
        data = {"purpose": EXPECTED_PURPOSE}

        response = requests.post(POST_FILE_URL, files=files, data=data, timeout=REQUEST_TIMEOUT)

        log_response(f"Status Code: {response.status_code}", test_name)
        log_response(f"Response Body: {response.text}", test_name)

        # Empty files are usually acceptable. Expect 200 OK.
        assert_equal(response.status_code, 200)
        try:
            json_data = response.json()
            assert_equal(json_data.get("bytes"), 0)
            assert_equal(json_data.get("purpose"), EXPECTED_PURPOSE)
            assert_equal(json_data.get("object"), EXPECTED_OBJECT)
            assert isinstance(json_data.get("id"), str)
        except (requests.exceptions.JSONDecodeError, AssertionError) as e:
             pytest.fail(f"Validation failed for empty file upload: {e}\nResponse: {response.text}", pytrace=False)

    def test_api_create_file_empty_purpose_string(self, tmp_path):
        """Verify API handles 'purpose' field being an empty string."""
        test_name = "test_api_create_file_empty_purpose_string"
        file_path = tmp_path / "empty_purpose_val.txt"
        file_path.write_bytes(b"content")

        with open(file_path, "rb") as file:
            files = {"file": (file_path.name, file, "text/plain")}
            data = {"purpose": ""} # Empty string for purpose
            response = requests.post(POST_FILE_URL, files=files, data=data, timeout=REQUEST_TIMEOUT)

            log_response(f"Status Code: {response.status_code}", test_name)
            log_response(f"Response Body: {response.text}", test_name)

        # Expecting a client error (400 Bad Request) as "" is not a valid purpose.
        assert 400 <= response.status_code < 500, f"Expected 4xx status code for empty purpose, got {response.status_code}"
        assert not is_server_error(response.status_code), "Server error occurred"

    # ---- Tests for Incorrect Request Structure ----


    @pytest.mark.parametrize("content_type", [
        "application/json",
        "text/plain",
        "application/xml",
        "application/x-www-form-urlencoded",
        None # Test missing Content-Type header
    ])
    def test_api_create_file_invalid_content_type(self, content_type):
        """Verify API rejects requests with incorrect Content-Type header."""
        test_name = f"test_api_create_file_invalid_content_type_{content_type or 'None'}"
        log_response(f"Testing Content-Type: {content_type}", test_name)

        headers = {}
        if content_type:
            headers["Content-Type"] = content_type

        # Send some dummy data appropriate for the fake content type if needed
        data_to_send = '{"purpose": "assistants", "file": "dummy"}' if content_type == "application/json" else "dummy data"

        # Use data= instead of files= since we are not sending multipart
        response = requests.post(POST_FILE_URL, headers=headers, data=data_to_send, timeout=REQUEST_TIMEOUT)

        log_response(f"Status Code: {response.status_code}", test_name)
        log_response(f"Response Body: {response.text}", test_name)

        # Expect 415 Unsupported Media Type or potentially 400 Bad Request.
        assert response.status_code in [400, 415], f"Expected 400 or 415 status code, got {response.status_code}"
        assert not is_server_error(response.status_code), "Server error occurred"


    @pytest.mark.parametrize("path", [
        "/v1/file",             # Singular instead of plural
        "/v1/files/",           # Trailing slash (might be treated differently)
        "/v1/files/some-id",    # Looks like a specific resource path
        "/v2/files",            # Incorrect version
        "/files"                # Missing base path/version
    ])

    def test_api_create_file_incorrect_path(self, path):
        """Verify requests to incorrect paths are rejected."""
        test_name = f"test_api_create_file_incorrect_path_{path.replace('/', '_')}"
        full_url = f"http://127.0.0.1:3928{path}" # Construct full URL
        log_response(f"Testing incorrect path: {full_url}", test_name)

        # Use dummy data/files, the path is the focus
        file_obj = io.BytesIO(b"dummy")
        files = {"file": ("dummy.txt", file_obj, "application/octet-stream")}
        data = {"purpose": EXPECTED_PURPOSE}

        try:
            response = requests.post(full_url, files=files, data=data, timeout=REQUEST_TIMEOUT)
        except requests.exceptions.RequestException as e:
            # Connection errors are possible if base path is totally wrong
            log_response(f"Request failed for path {path}: {e}", test_name)
            return # Don't assert on connection errors

        log_response(f"Status Code: {response.status_code}", test_name)
        log_response(f"Response Body: {response.text}", test_name)

        # Expect 404 Not Found.
        assert_equal(response.status_code, 404)
        assert not is_server_error(response.status_code), f"Server error for path {path}"

    # ---- Test for Large File (can be slow) ----
    # Mark this test as 'slow' using pytest markers if needed:
    # @pytest.mark.slow
    @pytest.mark.skip(reason="Test requires significant resources/time, enable manually if needed")
    def test_api_create_file_very_large_file(self, tmp_path):
        """Verify API handles very large files (e.g., > 100MB). SKIPPED by default."""
        test_name = "test_api_create_file_very_large_file"
        # Define large size (e.g., 100 MB)
        large_size_bytes = 100 * 1024 * 1024
        large_file_path = tmp_path / "large_file.bin"

        log_response(f"Creating large file ({large_size_bytes / (1024*1024):.1f} MB)...", test_name)
        try:
            with open(large_file_path, "wb") as f:
                # Write in chunks to manage memory, though seek is faster for sparse files
                # For simplicity here, just seek and write a byte at the end
                # Note: This creates a sparse file on filesystems that support it (like ext4, NTFS)
                # which takes up little actual disk space but reports the large size.
                # If the server reads the whole declared size, this test is still valid.
                f.seek(large_size_bytes - 1)
                f.write(b"\0")
            log_response(f"Large file created: {large_file_path}", test_name)

            file_size_on_disk = large_file_path.stat().st_size
            assert file_size_on_disk == large_size_bytes, f"Created file size mismatch: {file_size_on_disk}"

            with open(large_file_path, "rb") as file:
                files = {"file": (large_file_path.name, file, "application/octet-stream")}
                data = {"purpose": EXPECTED_PURPOSE}
                # Increase timeout significantly for large uploads
                large_file_timeout = 300 # 5 minutes

                log_response(f"Attempting upload...", test_name)
                response = requests.post(POST_FILE_URL, files=files, data=data, timeout=large_file_timeout)

            log_response(f"Status Code: {response.status_code}", test_name)
            # Avoid logging potentially huge response body
            log_response(f"Response Length: {len(response.text)}", test_name)


            assert not is_server_error(response.status_code), "Server error occurred processing large file"

            if response.status_code == 200:
                log_response("Large file uploaded successfully.", test_name)
                json_data = response.json()
                assert_equal(json_data.get("bytes"), large_size_bytes)
                assert_equal(json_data.get("purpose"), EXPECTED_PURPOSE)
            elif response.status_code == 413:
                log_response("Server rejected large file (413 Payload Too Large) - This may be expected.", test_name)
                # This is an acceptable outcome if the server has limits.
            else:
                pytest.fail(f"Unexpected status code {response.status_code} for large file upload.", pytrace=False)

        except MemoryError:
             pytest.skip("Skipping large file test due to insufficient memory.")
        except requests.exceptions.Timeout:
             pytest.fail("Request timed out during large file upload.", pytrace=False)
        except requests.exceptions.RequestException as e:
             pytest.fail(f"Request failed during large file upload: {e}", pytrace=False)
        finally:
             # Clean up the large file if it was created
             if large_file_path.exists():
                 try:
                     large_file_path.unlink()
                 except OSError as e:
                     log_response(f"Warning: Failed to delete large temp file {large_file_path}: {e}", test_name)

    # ----- Security Tests -----

    @pytest.mark.parametrize("malicious_filename", [
        "../sensitive.conf",
        "test/../../etc/passwd",
        "..\\windows\\system32\\config", # Windows style added for coverage
        "....//tricky.txt",
        "file/name/with/../in/middle.txt",
        "/absolute/path/../file.txt",
        "nul../file.txt",
        "file.txt..",
        "..file.txt"
    ])
    def test_api_create_file_path_traversal_filename(self, malicious_filename):
        """Verify API rejects filenames attempting path traversal using '..'."""
        sanitized_part = malicious_filename.replace('/', '_').replace('\\', '_').replace('.', '_')
        test_name = f"test_api_create_file_path_traversal_filename_{sanitized_part}"
        log_response(f"Testing potentially malicious filename: {malicious_filename!r}", test_name)

        file_content = b"Path traversal attempt"
        file_obj = io.BytesIO(file_content)

        files = {"file": (malicious_filename, file_obj, "application/octet-stream")}
        data = {"purpose": EXPECTED_PURPOSE}

        try:
            response = requests.post(POST_FILE_URL, files=files, data=data, timeout=REQUEST_TIMEOUT)
        except requests.exceptions.RequestException as e:
             log_response(f"Request failed client-side for filename {malicious_filename!r}: {e}", test_name)
             pytest.fail(f"Request failed before reaching server for filename {malicious_filename!r}", pytrace=False)

        log_response(f"Status Code: {response.status_code}", test_name)
        log_response(f"Response Body (first 500 chars): {response.text[:500]}...", test_name)

        assert 400 <= response.status_code < 500, \
            f"Expected 4xx status code for malicious filename '{malicious_filename}', got {response.status_code}"
        assert not is_server_error(response.status_code), f"Server error occurred for filename {malicious_filename}"


    # ----- Fuzzing Tests using Hypothesis (from previous response) -----

    HYPOTHESIS_SETTINGS = settings(
        deadline=None, # Allow more time for network requests
        suppress_health_check=[HealthCheck.too_slow, HealthCheck.data_too_large],
        max_examples=50 # Adjust as needed
    )

    fuzzy_filenames = st.text(
        alphabet=st.characters(
            min_codepoint=1,
            max_codepoint=0x10FFFF, # New: Full range including Supplementary Planes

            blacklist_categories=(
                'Cs',
                'Cc',
            ),
            blacklist_characters='\\/\0' # Blacklist slashes, backslashes, null bytes
        ),
        min_size=1,
        max_size=255 # Common filesystem limit
    )

    @HYPOTHESIS_SETTINGS
    @given(filename=fuzzy_filenames)
    def test_fuzz_filename(self, filename):
        """Tests uploading a file with various generated filenames."""
        test_name = "test_fuzz_filename"
        log_response(f"Fuzzing with filename: {filename!r}", test_name)
        file_obj = io.BytesIO(b"Fuzz test content")
        files = {"file": (filename, file_obj, "application/octet-stream")}
        data = {"purpose": EXPECTED_PURPOSE}

        try:
            response = requests.post(POST_FILE_URL, files=files, data=data, timeout=REQUEST_TIMEOUT)
            log_response(f"Status: {response.status_code}", test_name)
            log_response(f"Response head: {response.text[:100]}...", test_name)
        except requests.exceptions.RequestException as e:
            log_response(f"Request failed: {e}", test_name)
            return

        # This fuzz test checks broader filename handling, not just path traversal which is tested above.
        # A strict check might be: assert response.status_code == 200
        # However, some generated filenames might be legitimately invalid (e.g., too long if server enforces < 255, specific chars).
        # Therefore, only check for server errors which indicate unexpected crashes.
        # Client errors (4xx) might be acceptable for certain fuzz inputs.
        assert not is_server_error(response.status_code), \
            f"Server error ({response.status_code}) for filename: {filename!r}"


    fuzzy_purposes = st.one_of(
        st.none(), st.booleans(), st.integers(),
        st.floats(allow_nan=False, allow_infinity=False),
        st.text(max_size=1024), st.binary(max_size=1024)
    )

    @HYPOTHESIS_SETTINGS
    @given(purpose=fuzzy_purposes)
    def test_fuzz_purpose(self, purpose):
        """Tests uploading a file with various generated 'purpose' values."""
        test_name = "test_fuzz_purpose"
        log_response(f"Fuzzing with purpose: {purpose!r}", test_name)
        file_obj = io.BytesIO(b"Purpose fuzz test")
        files = {"file": ("purpose_test.txt", file_obj, "text/plain")}
        if isinstance(purpose, bytes):
             # Try decoding bytes, replace errors if needed for data field
             data = {"purpose": purpose.decode('utf-8', errors='replace')}
        else:
             # Convert other types to string as multipart typically sends strings
             data = {"purpose": str(purpose)}

        try:
            response = requests.post(POST_FILE_URL, files=files, data=data, timeout=REQUEST_TIMEOUT)
            log_response(f"Status: {response.status_code}", test_name)
            log_response(f"Response head: {response.text[:100]}...", test_name)
        except requests.exceptions.RequestException as e:
            log_response(f"Request failed: {e}", test_name)
            return

        assert not is_server_error(response.status_code), \
            f"Server error ({response.status_code}) for purpose: {purpose!r}"

        # Check logic based on whether the fuzz input matches the expected valid purpose
        if str(purpose) == EXPECTED_PURPOSE:
            assert response.status_code == 200, \
                f"Expected 200 for valid purpose '{EXPECTED_PURPOSE}', got {response.status_code}"
        elif response.status_code == 200:
             # This might indicate the server is too lenient with 'purpose' validation
             log_response(f"WARNING: Received 200 OK for unexpected purpose: {purpose!r}", test_name)
        else: # Expecting 4xx for invalid purposes
            assert 400 <= response.status_code < 500, \
                f"Expected client error (4xx) for invalid purpose {purpose!r}, got {response.status_code}"

    fuzzy_content = st.binary(max_size=2048)

    @HYPOTHESIS_SETTINGS
    @given(content=fuzzy_content)
    def test_fuzz_file_content(self, content):
        """Tests uploading files with various binary content."""
        test_name = "test_fuzz_file_content"
        log_response(f"Fuzzing with file content length: {len(content)} bytes", test_name)
        file_obj = io.BytesIO(content)
        files = {"file": ("content_test.bin", file_obj, "application/octet-stream")}
        data = {"purpose": EXPECTED_PURPOSE}

        try:
            response = requests.post(POST_FILE_URL, files=files, data=data, timeout=REQUEST_TIMEOUT + 5) # Slightly longer timeout
            log_response(f"Status: {response.status_code}", test_name)
            log_response(f"Response head: {response.text[:100]}...", test_name)
        except requests.exceptions.RequestException as e:
            log_response(f"Request failed: {e}", test_name)
            return

        assert not is_server_error(response.status_code), \
            f"Server error ({response.status_code}) for content length: {len(content)}"

        if response.status_code == 200:
             try:
                 json_data = response.json()
                 assert_equal(json_data.get("bytes"), len(content))
                 assert_equal(json_data.get("purpose"), EXPECTED_PURPOSE)
             except (requests.exceptions.JSONDecodeError, AssertionError, TypeError) as e:
                  pytest.fail(f"Validation failed for fuzz content (len {len(content)}): {e}\nResponse: {response.text}", pytrace=False)


    @HYPOTHESIS_SETTINGS
    @given(
        filename=fuzzy_filenames,
        purpose_val=st.text(max_size=100),
        content=st.binary(max_size=512)
    )
    def test_fuzz_combined(self, filename, purpose_val, content):
        """Tests uploading files with combined variations of filename, purpose, and content."""
        test_name = "test_fuzz_combined"
        log_response(f"Fuzzing combined: fn={filename!r}, p={purpose_val!r}, len={len(content)}", test_name)
        file_obj = io.BytesIO(content)
        files = {"file": (filename, file_obj, "application/octet-stream")}
        data = {"purpose": purpose_val}

        try:
            response = requests.post(POST_FILE_URL, files=files, data=data, timeout=REQUEST_TIMEOUT + 5)
            log_response(f"Status: {response.status_code}", test_name)
        except requests.exceptions.RequestException as e:
            log_response(f"Request failed: {e}", test_name)
            return

        assert not is_server_error(response.status_code), \
            f"Server error ({response.status_code}) for combined input: fn={filename!r}, p={purpose_val!r}, len={len(content)}"

        # Basic logic checks, might need refinement based on specific API rules
        if purpose_val == EXPECTED_PURPOSE and response.status_code != 200:
             log_response(f"WARNING: Expected 200 for valid purpose but got {response.status_code} in combined test.", test_name)
        elif purpose_val != EXPECTED_PURPOSE and not (400 <= response.status_code < 500):
             # If purpose is invalid, expect 4xx. If we get 200 or 5xx, log a warning/potentially fail.
             # Allow 200 if *maybe* the filename or content caused a specific override? Less likely.
             # Focus on avoiding 5xx primarily.
             if response.status_code == 200:
                 log_response(f"WARNING: Received 200 OK for invalid purpose '{purpose_val!r}' in combined test.", test_name)
             else: # Not 200, not 4xx, not 5xx (already asserted above) -> Unexpected status code
                 log_response(f"INFO: Unexpected status {response.status_code} for invalid purpose '{purpose_val!r}' in combined test.", test_name)
