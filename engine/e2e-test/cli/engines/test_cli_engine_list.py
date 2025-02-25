import platform

import pytest
from utils.test_runner import run, start_server, stop_server


class TestCliEngineList:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup TODO(sang) should make all the test isolate
        stop_server()
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    @pytest.mark.skipif(platform.system() != "Windows", reason="Windows-specific test")
    def test_engines_list_run_successfully_on_windows(self):
        exit_code, output, error = run("List engines", ["engines", "list"])
        assert exit_code == 0, f"List engines failed with error: {error}"

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_list_run_successfully_on_macos(self):
        exit_code, output, error = run("List engines", ["engines", "list"])
        assert exit_code == 0, f"List engines failed with error: {error}"

    @pytest.mark.skipif(platform.system() != "Linux", reason="Linux-specific test")
    def test_engines_list_run_successfully_on_linux(self):
        exit_code, output, error = run("List engines", ["engines", "list"])
        assert exit_code == 0, f"List engines failed with error: {error}"
