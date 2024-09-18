import platform

import pytest
from test_runner import run


class TestCliEngineList:
    @pytest.mark.skipif(platform.system() != "Windows", reason="Windows-specific test")
    def test_engines_list_run_successfully_on_windows(self):
        exit_code, output, error = run("List engines", ["engines", "list"])
        assert exit_code == 0, f"List engines failed with error: {error}"
        assert "llama.cpp" in output

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_list_run_successfully_on_macos(self):
        exit_code, output, error = run("List engines", ["engines", "list"])
        assert exit_code == 0, f"List engines failed with error: {error}"
        assert "llama.cpp" in output

    @pytest.mark.skipif(platform.system() != "Linux", reason="Linux-specific test")
    def test_engines_list_run_successfully_on_linux(self):
        exit_code, output, error = run("List engines", ["engines", "list"])
        assert exit_code == 0, f"List engines failed with error: {error}"
        assert "llama.cpp" in output