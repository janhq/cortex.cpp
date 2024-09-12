import platform

import pytest
from test_runner import run


class TestCliEngineInstall:

    def test_engines_install_llamacpp_should_be_successfully(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "cortex.llamacpp"]
        )
        assert "Download" in output, "Should display downloading message"
        assert exit_code == 0, f"Install engine failed with error: {error}"

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_install_onnx_on_macos_should_be_failed(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "cortex.onnx"]
        )
        assert "No variant found" in output, "Should display error message"
        assert exit_code == 0, f"Install engine failed with error: {error}"

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_install_onnx_on_tensorrt_should_be_failed(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "cortex.tensorrt-llm"]
        )
        assert "No variant found" in output, "Should display error message"
        assert exit_code == 0, f"Install engine failed with error: {error}"
