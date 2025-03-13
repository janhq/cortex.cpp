import platform
import tempfile

import pytest
import requests
from utils.test_runner import run, start_server, stop_server, get_latest_pre_release_tag

latest_pre_release_tag = get_latest_pre_release_tag("janhq", "cortex.llamacpp")

class TestCliEngineInstall:
    def setup_and_teardown(self):
        # Setup
        stop_server()
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_engines_install_llamacpp_should_be_successfully(self):
        exit_code, output, error = run(
            "Install Engine",
            ["engines", "install", "llama-cpp"],
            timeout=None,
            capture=False,
        )
        response = requests.get("http://127.0.0.1:3928/v1/engines/llama-cpp")
        assert len(response.json()) > 0
        assert exit_code == 0, f"Install engine failed with error: {error}"

    def test_engines_install_python_should_be_successfully(self):
        exit_code, output, error = run(
            "Install Engine",
            ["engines", "install", "python-engine"],
            timeout=None,
            capture=False,
        )
        response = requests.get("http://127.0.0.1:3928/v1/engines/python-engine")
        assert len(response.json()) > 0
        assert exit_code == 0, f"Install engine failed with error: {error}"

    @pytest.mark.skipif(reason="Ignore onnx-runtime test")
    def test_engines_install_onnx_on_macos_should_be_failed(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "onnxruntime"]
        )
        assert "is not supported on" in output, "Should display error message"
        assert exit_code == 0, f"Install engine failed with error: {error}"

    @pytest.mark.skipif(reason="Ignore tensorrt-llm test")
    def test_engines_install_onnx_on_tensorrt_should_be_failed(self):
        exit_code, output, error = run(
            "Install Engine", ["engines", "install", "tensorrt-llm"]
        )
        assert "is not supported on" in output, "Should display error message"
        assert exit_code == 0, f"Install engine failed with error: {error}"

    @pytest.mark.skipif(platform.system() == "Linux", reason="Wait for linux arm ready")
    def test_engines_should_fallback_to_download_llamacpp_engine_if_not_exists(self):
        exit_code, output, error = run(
            "Install Engine",
            ["engines", "install", "llama-cpp", "-s", tempfile.gettempdir()],
            timeout=None,
        )
        # response = requests.get("http://127.0.0.1:3928/v1/engines/llama-cpp")
        # assert len(response.json()) > 0
        assert "downloaded successfully" in output
        assert exit_code == 0, f"Install engine failed with error: {error}"

    def test_engines_should_not_perform_with_dummy_path(self):
        exit_code, output, error = run(
            "Install Engine",
            ["engines", "install", "llama-cpp", "-s", "abcpod"],
            timeout=None,
        )
        assert "Folder does not exist" in output, "Should display error"
        assert exit_code == 0, f"Install engine failed with error: {error}"

    def test_engines_install_pre_release_llamacpp(self):
        engine_version = latest_pre_release_tag
        exit_code, output, error = run(
            "Install Engine",
            ["engines", "install", "llama-cpp", "-v", engine_version],
            timeout=None,
            capture=False,
        )
        response = requests.get("http://127.0.0.1:3928/v1/engines/llama-cpp")
        assert len(response.json()) > 0
        is_engine_version_exist = False
        for item in response.json():
            # Check if 'version' key exists and matches target
            if "version" in item and item["version"] == engine_version:
                is_engine_version_exist = True
                break

        # loop through all the installed response, expect we find
        assert is_engine_version_exist, f"Engine version {engine_version} is not found"
        assert exit_code == 0, f"Install engine failed with error: {error}"
