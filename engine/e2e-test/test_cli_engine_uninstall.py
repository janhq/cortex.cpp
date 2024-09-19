import pytest
from test_runner import run


class TestCliEngineUninstall:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        # Preinstall llamacpp engine
        run("Install Engine", ["engines", "install", "cortex.llamacpp"],timeout=None)

        yield

        # Teardown
        # Clean up, removing installed engine
        run("Uninstall Engine", ["engines", "uninstall", "cortex.llamacpp"])

    def test_engines_uninstall_llamacpp_should_be_successfully(self):
        exit_code, output, error = run(
            "Uninstall engine", ["engines", "uninstall", "cortex.llamacpp"]
        )
        assert "Engine cortex.llamacpp uninstalled successfully!" in output
        assert exit_code == 0, f"Install engine failed with error: {error}"
