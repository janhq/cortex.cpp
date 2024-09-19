#include "model_get_cmd.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>
#include "cmd_info.h"
#include "config/yaml_config.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/modellist_utils.h"

namespace commands {

void ModelGetCmd::Exec(const std::string& model_handle) {
  modellist_utils::ModelListUtils modellist_handler;
  config::YamlHandler yaml_handler;
  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
    auto model_config = yaml_handler.GetModelConfig();

    // Helper function to print comments
    auto print_comment = [](const std::string& comment) {
      std::cout << "\033[1;90m# " << comment << "\033[0m\n";
    };

    print_comment("BEGIN GENERAL GGUF METADATA");

    // Helper function to print key-value pairs with color
    auto print_kv = [](const std::string& key, const auto& value,
                       const std::string& color = "\033[0m") {
      std::cout << "\033[1;32m" << key << ":\033[0m " << color << value
                << "\033[0m\n";
    };

    // Helper function to print boolean values
    auto print_bool = [&print_kv](const std::string& key, bool value) {
      print_kv(key, value ? "true" : "false", "\033[0;35m");
    };

    // Print non-empty strings
    if (!model_config.id.empty())
      print_kv("id", model_config.id, "\033[0;33m");
    if (!model_config.name.empty())
      print_kv("name", model_config.name, "\033[0;33m");
    if (!model_config.model.empty())
      print_kv("model", model_config.model, "\033[0;33m");
    if (!model_config.version.empty())
      print_kv("version", model_config.version, "\033[0;33m");

    // Print non-empty vectors
    if (!model_config.files.empty()) {
      std::cout << "\033[1;32mfiles:\033[0m\n";
      for (const auto& file : model_config.files) {
        std::cout << "  - \033[0;33m" << file << "\033[0m\n";
      }
    }

    print_comment("END GENERAL GGUF METADATA");
    print_comment("BEGIN INFERENCE PARAMETERS");
    print_comment("BEGIN REQUIRED");

    if (!model_config.stop.empty()) {
      std::cout << "\033[1;32mstop:\033[0m\n";
      for (const auto& stop : model_config.stop) {
        std::cout << "  - \033[0;33m" << stop << "\033[0m\n";
      }
    }

    print_comment("END REQUIRED");
    print_comment("BEGIN OPTIONAL");

    // Print boolean values
    print_bool("stream", model_config.stream);

    // Print float values with fixed precision
    auto print_float = [&print_kv](const std::string& key, float value) {
      if (!std::isnan(value)) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(9) << value;
        print_kv(key, oss.str(), "\033[0;34m");
      }
    };

    print_float("top_p", model_config.top_p);
    print_float("temperature", model_config.temperature);
    print_float("frequency_penalty", model_config.frequency_penalty);
    print_float("presence_penalty", model_config.presence_penalty);

    // Print integer values
    auto print_int = [&print_kv](const std::string& key, int value) {
      if (value != 0) {  // Assuming 0 is the default/unset value
        print_kv(key, value, "\033[0;35m");
      }
    };

    print_int("max_tokens", static_cast<int>(model_config.max_tokens));
    print_int("seed", model_config.seed);
    print_float("dynatemp_range", model_config.dynatemp_range);
    print_float("dynatemp_exponent", model_config.dynatemp_exponent);
    print_int("top_k", model_config.top_k);
    print_float("min_p", model_config.min_p);
    print_int("tfs_z", model_config.tfs_z);
    print_float("typ_p", model_config.typ_p);
    print_int("repeat_last_n", model_config.repeat_last_n);
    print_float("repeat_penalty", model_config.repeat_penalty);
    print_bool("mirostat", model_config.mirostat);
    print_float("mirostat_tau", model_config.mirostat_tau);
    print_float("mirostat_eta", model_config.mirostat_eta);
    print_bool("penalize_nl", model_config.penalize_nl);
    print_bool("ignore_eos", model_config.ignore_eos);
    print_int("n_probs", model_config.n_probs);
    print_int("min_keep", model_config.min_keep);

    print_comment("END OPTIONAL");
    print_comment("END INFERENCE PARAMETERS");
    print_comment("BEGIN MODEL LOAD PARAMETERS");
    print_comment("BEGIN REQUIRED");

    if (!model_config.engine.empty())
      print_kv("engine", model_config.engine, "\033[0;33m");
    if (!model_config.prompt_template.empty())
      print_kv("prompt_template", model_config.prompt_template, "\033[0;33m");

    print_comment("END REQUIRED");
    print_comment("BEGIN OPTIONAL");

    print_int("ctx_len", static_cast<int>(model_config.ctx_len));
    print_int("ngl", static_cast<int>(model_config.ngl));

    print_comment("END OPTIONAL");
    print_comment("END MODEL LOAD PARAMETERS");

  } catch (const std::exception& e) {
    CLI_LOG("Fail to get model information with ID '" + model_handle +
            "': " + e.what());
  }
}

}  // namespace commands