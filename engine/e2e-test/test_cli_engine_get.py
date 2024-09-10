import platform

import pytest
from test_runner import run


class TestCliEngineGet:
    @pytest.mark.skipif(platform.system() != "Windows", reason="Windows-specific test")
    def test_engines_list_run_successfully_on_windows(self):
        # TODO: implement
        assert 0 == 0

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_get_run_successfully_on_macos(self):
        exit_code, output, error = run("Get engine", ["engines", "get", "cortex.llamacpp"])
        assert exit_code == 0, f"Get engine failed with error: {error}"

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_get_llamacpp_on_macos(self):
        exit_code, output, error = run("Get engine", ["engines", "get", "cortex.llamacpp"])
        assert exit_code == 0, f"Get engine failed with error: {error}"
        assert "Incompatible" not in output, f"cortex.llamacpp can only be Ready or Not Installed"

    @pytest.mark.skipif(platform.system() != "Linux", reason="Linux-specific test")
    def test_engines_list_run_successfully_on_linux(self):
        # TODO: implement
        assert 0 == 0
