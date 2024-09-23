import platform
import tempfile

import pytest
from test_runner import run


class TestCliEngineInstall:

    def test_engines_install_llamacpp_should_be_successfully(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "cortex.llamacpp"], timeout=600
        )
        assert "Start downloading" in output, "Should display downloading message"
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
        
    def test_engines_install_pre_release_llamacpp(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "cortex.llamacpp", "-v", "v0.1.29"], timeout=600
        )
        assert "Start downloading" in output, "Should display downloading message"
        assert exit_code == 0, f"Install engine failed with error: {error}"

    def test_engines_should_fallback_to_download_llamacpp_engine_if_not_exists(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "cortex.llamacpp", "-s", tempfile.gettempdir()], timeout=None
        )
        assert "Start downloading" in output, "Should display downloading message"
        assert exit_code == 0, f"Install engine failed with error: {error}"
        
    def test_engines_should_not_perform_with_dummy_path(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "cortex.llamacpp", "-s", "abcpod"], timeout=None
        )
        assert "Folder does not exist" in output, "Should display error"
        assert exit_code == 0, f"Install engine failed with error: {error}"
