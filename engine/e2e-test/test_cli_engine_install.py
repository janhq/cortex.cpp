import platform
import tempfile
import os
from pathlib import Path
import pytest
from test_runner import run


class TestCliEngineInstall:
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_engines_install_llamacpp_should_be_successfully(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "llama-cpp"], timeout=None, capture = False
        )
        root = Path.home()
        assert os.path.exists(root / "cortexcpp" / "engines" / "cortex.llamacpp" / "version.txt")
        assert exit_code == 0, f"Install engine failed with error: {error}"

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_install_onnx_on_macos_should_be_failed(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "onnxruntime"]
        )
        assert "No variant found" in output, "Should display error message"
        assert exit_code == 0, f"Install engine failed with error: {error}"

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_install_onnx_on_tensorrt_should_be_failed(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "tensorrt-llm"]
        )
        assert "No variant found" in output, "Should display error message"
        assert exit_code == 0, f"Install engine failed with error: {error}"
        
    def test_engines_install_pre_release_llamacpp(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "llama-cpp", "-v", "v0.1.29"], timeout=None, capture = False
        )
        root = Path.home()
        assert os.path.exists(root / "cortexcpp" / "engines" / "cortex.llamacpp" / "version.txt")
        assert exit_code == 0, f"Install engine failed with error: {error}"

    def test_engines_should_fallback_to_download_llamacpp_engine_if_not_exists(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "llama-cpp", "-s", tempfile.gettempdir()], timeout=None
        )
        assert "Start downloading" in output, "Should display downloading message"
        assert exit_code == 0, f"Install engine failed with error: {error}"
        
    def test_engines_should_not_perform_with_dummy_path(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "llama-cpp", "-s", "abcpod"], timeout=None
        )
        assert "Folder does not exist" in output, "Should display error"
        assert exit_code == 0, f"Install engine failed with error: {error}"
