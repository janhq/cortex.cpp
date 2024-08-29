#include "engine_list_cmd.h"
#include <filesystem>
// clang-format off
#include <tabulate/table.hpp>
#include <utility>
#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
// clang-format on
namespace commands {

bool EngineListCmd::Exec() {
  tabulate::Table table;
  table.add_row(
      {"(Index)", "name", "description", "version", "product name", "status"});
  table.format().font_color(tabulate::Color::green);
#ifdef _WIN32
  if (std::filesystem::exists(std::filesystem::current_path().string() +
                              cortex_utils::kOnnxLibPath)) {
    table.add_row({"1", "cortex.onnx",
                   "This extension enables chat completion API calls using the "
                   "Onnx engine",
                   "0.0.1", "Onnx Inference Engine", "ready"});
  } else {
    table.add_row({"1", "cortex.onnx",
                   "This extension enables chat completion API calls using the "
                   "Onnx engine",
                   "0.0.1", "Onnx Inference Engine", "not_initialized"});
  }

#else
  table.add_row(
      {"1", "cortex.onnx",
       "This extension enables chat completion API calls using the Onnx engine",
       "0.0.1", "Onnx Inference Engine", "not_supported"});
#endif
  // lllamacpp
  if (std::filesystem::exists(std::filesystem::current_path().string() +
                              cortex_utils::kLlamaLibPath)) {
    table.add_row({"2", "cortex.llamacpp",
                   "This extension enables chat completion API calls using the "
                   "LlamaCPP engine",
                   "0.0.1", "LlamaCPP Inference Engine", "ready"});
  } else {
    table.add_row({"2", "cortex.llamacpp",
                   "This extension enables chat completion API calls using the "
                   "LlamaCPP engine",
                   "0.0.1", "LlamaCPP Inference Engine", "not_initialized"});
  }
  // tensorrt llm
  if (std::filesystem::exists(std::filesystem::current_path().string() +
                              cortex_utils::kTensorrtLlmPath)) {
    table.add_row({"3", "cortex.tensorrt-llm",
                   "This extension enables chat completion API calls using the "
                   "TensorrtLLM engine",
                   "0.0.1", "TensorrtLLM Inference Engine", "ready"});
  } else {
    table.add_row({"3", "cortex.tensorrt-llm",
                   "This extension enables chat completion API calls using the "
                   "TensorrtLLM engine",
                   "0.0.1", "TensorrtLLM Inference Engine", "not_initialized"});
  }
  for (int i = 0; i < 6; i++) {
    table[0][i]
        .format()
        .font_color(tabulate::Color::white)  // Set font color
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center);
  }
  for (int i = 1; i < 4; i++) {
    table[i][0]
        .format()
        .font_color(tabulate::Color::white)  // Set font color
        .font_align(tabulate::FontAlign::center);
  }

  std::cout << table << std::endl;
  return true;
}

};  // namespace commands