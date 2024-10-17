import pytest
from test_runner import run
from test_runner import start_server, stop_server

class TestCliEngineUninstall:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        # Preinstall llamacpp engine
        run("Install Engine", ["engines", "install", "llama-cpp"],timeout = None)

        yield

        # Teardown
        # Clean up, removing installed engine
        run("Uninstall Engine", ["engines", "uninstall", "llama-cpp"])
        stop_server()

    def test_engines_uninstall_llamacpp_should_be_successfully(self):
        exit_code, output, error = run(
            "Uninstall engine", ["engines", "uninstall", "llama-cpp"]
        )
        assert "Engine llama-cpp uninstalled successfully!" in output
        assert exit_code == 0, f"Install engine failed with error: {error}"
