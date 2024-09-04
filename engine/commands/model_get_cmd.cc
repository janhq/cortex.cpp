#include "model_get_cmd.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include "cmd_info.h"
#include "config/yaml_config.h"
#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

namespace commands {

ModelGetCmd::ModelGetCmd(std::string model_handle)
    : model_handle_(std::move(model_handle)) {}

void ModelGetCmd::Exec() {
  if (std::filesystem::exists(cortex_utils::models_folder) &&
      std::filesystem::is_directory(cortex_utils::models_folder)) {
    CmdInfo ci(model_handle_);
    std::string model_file =
        ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
    bool found_model = false;
    // Iterate through directory
    for (const auto& entry :
         std::filesystem::directory_iterator(cortex_utils::models_folder)) {

      if (entry.is_regular_file() && entry.path().stem() == model_file &&
          entry.path().extension() == ".yaml") {
        try {
          config::YamlHandler handler;
          handler.ModelConfigFromFile(entry.path().string());
          const auto& model_config = handler.GetModelConfig();
          std::cout << "ModelConfig Details:\n";
          std::cout << "-------------------\n";

          // Print non-null strings
          if (!model_config.id.empty())
            std::cout << "id: " << model_config.id << "\n";
          if (!model_config.name.empty())
            std::cout << "name: " << model_config.name << "\n";
          if (!model_config.model.empty())
            std::cout << "model: " << model_config.model << "\n";
          if (!model_config.version.empty())
            std::cout << "version: " << model_config.version << "\n";

          // Print non-empty vectors
          if (!model_config.stop.empty()) {
            std::cout << "stop: [";
            for (size_t i = 0; i < model_config.stop.size(); ++i) {
              std::cout << model_config.stop[i];
              if (i < model_config.stop.size() - 1)
                std::cout << ", ";
            }
            std::cout << "]\n";
          }
          // Print valid numbers
          if (!std::isnan(static_cast<double>(model_config.top_p)))
            std::cout << "top_p: " << model_config.top_p << "\n";
          if (!std::isnan(static_cast<double>(model_config.temperature)))
            std::cout << "temperature: " << model_config.temperature << "\n";
          if (!std::isnan(static_cast<double>(model_config.frequency_penalty)))
            std::cout << "frequency_penalty: " << model_config.frequency_penalty
                      << "\n";
          if (!std::isnan(static_cast<double>(model_config.presence_penalty)))
            std::cout << "presence_penalty: " << model_config.presence_penalty
                      << "\n";
          if (!std::isnan(static_cast<double>(model_config.max_tokens)))
            std::cout << "max_tokens: " << model_config.max_tokens << "\n";
          if (!std::isnan(static_cast<double>(model_config.stream)))

            std::cout << "stream: " << std::boolalpha << model_config.stream
                      << "\n";
          if (!std::isnan(static_cast<double>(model_config.ngl)))
            std::cout << "ngl: " << model_config.ngl << "\n";
          if (!std::isnan(static_cast<double>(model_config.ctx_len)))
            std::cout << "ctx_len: " << model_config.ctx_len << "\n";

          // Print non-null strings
          if (!model_config.engine.empty())
            std::cout << "engine: " << model_config.engine << "\n";
          if (!model_config.prompt_template.empty())

            std::cout << "prompt_template: " << model_config.prompt_template
                      << "\n";
          if (!model_config.system_template.empty())
            std::cout << "system_template: " << model_config.system_template
                      << "\n";
          if (!model_config.user_template.empty())
            std::cout << "user_template: " << model_config.user_template
                      << "\n";
          if (!model_config.ai_template.empty())
            std::cout << "ai_template: " << model_config.ai_template << "\n";
          if (!model_config.os.empty())
            std::cout << "os: " << model_config.os << "\n";
          if (!model_config.gpu_arch.empty())
            std::cout << "gpu_arch: " << model_config.gpu_arch << "\n";
          if (!model_config.quantization_method.empty())

            std::cout << "quantization_method: "
                      << model_config.quantization_method << "\n";
          if (!model_config.precision.empty())
            std::cout << "precision: " << model_config.precision << "\n";

          if (!std::isnan(static_cast<double>(model_config.tp)))
            std::cout << "tp: " << model_config.tp << "\n";

          // Print non-null strings
          if (!model_config.trtllm_version.empty())

            std::cout << "trtllm_version: " << model_config.trtllm_version
                      << "\n";
          if (!std::isnan(static_cast<double>(model_config.text_model)))
            std::cout << "text_model: " << std::boolalpha
                      << model_config.text_model << "\n";

          // Print non-empty vectors
          if (!model_config.files.empty()) {
            std::cout << "files: [";
            for (size_t i = 0; i < model_config.files.size(); ++i) {
              std::cout << model_config.files[i];
              if (i < model_config.files.size() - 1)
                std::cout << ", ";
            }
            std::cout << "]\n";
          }

          // Print valid size_t number
          if (model_config.created != 0)
            std::cout << "created: " << model_config.created << "\n";

          if (!model_config.object.empty())
            std::cout << "object: " << model_config.object << "\n";
          if (!model_config.owned_by.empty())
            std::cout << "owned_by: " << model_config.owned_by << "\n";

          found_model = true;
          break;
        } catch (const std::exception& e) {
          CTL_ERR("Error reading yaml file '" << entry.path().string()
                    << "': " << e.what());
        }
      }
    }
    if (!found_model) {
      CLI_LOG("Model not found!");
    }
  } else {
    CLI_LOG("Model not found!");
  }
}
};  // namespace commands