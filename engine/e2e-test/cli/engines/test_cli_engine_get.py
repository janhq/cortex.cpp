import platform

import pytest
from utils.test_runner import run, start_server, stop_server


class TestCliEngineGet:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    @pytest.mark.skipif(platform.system() != "Windows", reason="Windows-specific test")
    def test_engines_get_tensorrt_llm_should_not_be_incompatible(self):
        exit_code, output, error = run("Get engine", ["engines", "get", "tensorrt-llm"])
        assert exit_code == 0, f"Get engine failed with error: {error}"
        assert (
            "Incompatible" not in output
        ), "tensorrt-llm should be Ready or Not Installed on Windows"

    @pytest.mark.skipif(platform.system() != "Windows", reason="Windows-specific test")
    def test_engines_get_onnx_should_not_be_incompatible(self):
        exit_code, output, error = run("Get engine", ["engines", "get", "onnxruntime"])
        assert exit_code == 0, f"Get engine failed with error: {error}"
        assert (
            "Incompatible" not in output
        ), "onnxruntime should be Ready or Not Installed on Windows"

    def test_engines_get_llamacpp_should_not_be_incompatible(self):
        exit_code, output, error = run("Get engine", ["engines", "get", "llama-cpp"])
        assert exit_code == 0, f"Get engine failed with error: {error}"
        assert (
            "Incompatible" not in output
        ), "llama-cpp should be compatible for Windows, MacOs and Linux"

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_get_tensorrt_llm_should_be_incompatible_on_macos(self):
        exit_code, output, error = run("Get engine", ["engines", "get", "tensorrt-llm"])
        assert exit_code == 0, f"Get engine failed with error: {error}"
        assert (
            "is not supported on" in output
        ), "tensorrt-llm should be Incompatible on MacOS"

    @pytest.mark.skipif(platform.system() != "Darwin", reason="macOS-specific test")
    def test_engines_get_onnx_should_be_incompatible_on_macos(self):
        exit_code, output, error = run("Get engine", ["engines", "get", "onnxruntime"])
        assert exit_code == 0, f"Get engine failed with error: {error}"
        assert (
            "is not supported on" in output
        ), "onnxruntime should be Incompatible on MacOS"

    @pytest.mark.skipif(platform.system() != "Linux", reason="Linux-specific test")
    def test_engines_get_onnx_should_be_incompatible_on_linux(self):
        exit_code, output, error = run("Get engine", ["engines", "get", "onnxruntime"])
        print(output)
        assert exit_code == 0, f"Get engine failed with error: {error}"
        assert (
            "is not supported o" in output
        ), "onnxruntime should be Incompatible on Linux"
