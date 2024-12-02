#pragma once

constexpr const auto kOnnxEngine = "onnxruntime";
constexpr const auto kLlamaEngine = "llama-cpp";
constexpr const auto kTrtLlmEngine = "tensorrt-llm";

constexpr const auto kOnnxRepo = "cortex.onnx";
constexpr const auto kLlamaRepo = "cortex.llamacpp";
constexpr const auto kTrtLlmRepo = "cortex.tensorrt-llm";
constexpr const auto kPythonRuntimeRepo = "cortex.python";

constexpr const auto kLlamaLibPath = "./engines/cortex.llamacpp";
constexpr const auto kPythonRuntimeLibPath = "/engines/cortex.python";
constexpr const auto kOnnxLibPath = "/engines/cortex.onnx";
constexpr const auto kTensorrtLlmPath = "/engines/cortex.tensorrt-llm";

// other constants
constexpr auto static kHuggingFaceHost = "huggingface.co";
constexpr auto static kGitHubHost = "api.github.com";
constexpr auto static kCortexFolderName = "cortexcpp";
constexpr auto static kDefaultGHUserAgent = "cortexcpp";

constexpr auto static kWindowsOs = "windows";
constexpr auto static kMacOs = "mac";
constexpr auto static kLinuxOs = "linux";
constexpr auto static kUnsupportedOs = "Unsupported OS";

constexpr auto static kCurlGetTimeout = 10;
