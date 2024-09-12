#include "engine_service.h"
#include <stdexcept>
#include "algorithm"
#include "utils/file_manager_utils.h"

namespace {
constexpr static auto kIncompatible = "Incompatible";
constexpr static auto kReady = "Ready";
constexpr static auto kNotInstalled = "Not Installed";
}  // namespace

EngineInfo EngineService::GetEngineInfo(const std::string& engine) const {
  // if engine is not found in kSupportEngine, throw runtime error
  if (std::find(kSupportEngines.begin(), kSupportEngines.end(), engine) ==
      kSupportEngines.end()) {
    // TODO: create a custom exception class
    throw std::runtime_error("Engine " + engine + " is not supported!");
  }

  auto engine_status_list = GetEngineInfoList();

  return *std::find_if(
      engine_status_list.begin(), engine_status_list.end(),
      [&engine](const EngineInfo& e) { return e.name == engine; });
}

std::vector<EngineInfo> EngineService::GetEngineInfoList() const {
  auto ecp = file_manager_utils::GetEnginesContainerPath();

  std::string onnx_status{kIncompatible};
  std::string llamacpp_status =
      std::filesystem::exists(ecp / "cortex.llamacpp") ? kReady : kNotInstalled;
  std::string tensorrt_status{kIncompatible};

#ifdef _WIN32
  onnx_status =
      std::filesystem::exists(ecp / "cortex.onnx") ? kReady : kNotInstalled;
  tensorrt_status = std::filesystem::exists(ecp / "cortex.tensorrt-llm")
                        ? kReady
                        : kNotInstalled;
#elif defined(__linux__)
  tensorrt_status = std::filesystem::exists(ecp / "cortex.tensorrt-llm")
                        ? kReady
                        : kNotInstalled;
#endif
  std::vector<EngineInfo> engines = {
      {.name = "cortex.onnx",
       .description = "This extension enables chat completion API calls using "
                      "the Onnx engine",
       .format = "ONNX",
       .version = "0.0.1",
       .product_name = "ONNXRuntime",
       .status = onnx_status},
      {.name = "cortex.llamacpp",
       .description = "This extension enables chat completion API calls using "
                      "the LlamaCPP engine",
       .format = "GGUF",
       .version = "0.0.1",
       .product_name = "llama.cpp",
       .status = llamacpp_status},
      {.name = "cortex.tensorrt-llm",
       .description = "This extension enables chat completion API calls using "
                      "the TensorrtLLM engine",
       .format = "TensorRT Engines",
       .version = "0.0.1",
       .product_name = "TensorRT-LLM",
       .status = tensorrt_status},
  };

  return engines;
}

void EngineService::UninstallEngine(const std::string& engine) {
  CTL_INF("Uninstall engine " + engine);

  // TODO: Unload the model which is currently running on engine_

  // TODO: Unload engine if is loaded

  auto ecp = file_manager_utils::GetEnginesContainerPath();
  auto engine_path = ecp / engine;

  if (!std::filesystem::exists(engine_path)) {
    throw std::runtime_error("Engine " + engine + " is not installed!");
  }

  try {
    std::filesystem::remove_all(engine_path);
    CTL_INF("Engine " << engine << " uninstalled successfully!");
  } catch (const std::exception& e) {
    CTL_ERR("Failed to uninstall engine " << engine << ": " << e.what());
    throw;
  }
}
