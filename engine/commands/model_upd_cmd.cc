#include "model_upd_cmd.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {

ModelUpdCmd::ModelUpdCmd(std::string model_handle)
    : model_handle_(std::move(model_handle)) {}

void ModelUpdCmd::Exec(
    const std::unordered_map<std::string, std::string>& options) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  try {
    auto model_entry = model_list_utils_.GetModelInfo(model_handle_);
    if (model_entry.has_error()) {
      CLI_LOG("Error: " + model_entry.error());
      return;
    }
    auto yaml_fp =
        fmu::GetAbsolutePath(fmu::GetCortexDataPath(),
                             fs::path(model_entry.value().path_to_model_yaml));
    yaml_handler_.ModelConfigFromFile(yaml_fp.string());
    model_config_ = yaml_handler_.GetModelConfig();

    for (const auto& [key, value] : options) {
      if (!value.empty()) {
        UpdateConfig(key, value);
      }
    }

    yaml_handler_.UpdateModelConfig(model_config_);
    yaml_handler_.WriteYamlFile(yaml_fp.string());
    CLI_LOG("Successfully updated model ID '" + model_handle_ + "'!");
  } catch (const std::exception& e) {
    CLI_LOG("Failed to update model with model ID '" + model_handle_ +
            "': " + e.what());
  }
}

void ModelUpdCmd::UpdateConfig(const std::string& key,
                               const std::string& value) {
  static const std::unordered_map<
      std::string,
      std::function<void(ModelUpdCmd*, const std::string&, const std::string&)>>
      updaters = {
          {"name",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.name = v;
           }},
          {"model",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.model = v;
           }},
          {"version",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.version = v;
           }},
          {"engine",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.engine = v;
           }},
          {"prompt_template",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.prompt_template = v;
           }},
          {"system_template",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.system_template = v;
           }},
          {"user_template",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.user_template = v;
           }},
          {"ai_template",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.ai_template = v;
           }},
          {"os",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.os = v;
           }},
          {"gpu_arch",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.gpu_arch = v;
           }},
          {"quantization_method",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.quantization_method = v;
           }},
          {"precision",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.precision = v;
           }},
          {"trtllm_version",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.trtllm_version = v;
           }},
          {"object",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.object = v;
           }},
          {"owned_by",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.owned_by = v;
           }},
          {"grammar",
           [](ModelUpdCmd* self, const std::string&, const std::string& v) {
             self->model_config_.grammar = v;
           }},
          {"stop", &ModelUpdCmd::UpdateVectorField},
          {"files", &ModelUpdCmd::UpdateVectorField},
          {"top_p",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(
                 k, v, [self](float f) { self->model_config_.top_p = f; });
           }},
          {"temperature",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.temperature = f;
             });
           }},
          {"frequency_penalty",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.frequency_penalty = f;
             });
           }},
          {"presence_penalty",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.presence_penalty = f;
             });
           }},
          {"dynatemp_range",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.dynatemp_range = f;
             });
           }},
          {"dynatemp_exponent",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.dynatemp_exponent = f;
             });
           }},
          {"min_p",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(
                 k, v, [self](float f) { self->model_config_.min_p = f; });
           }},
          {"tfs_z",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(
                 k, v, [self](float f) { self->model_config_.tfs_z = f; });
           }},
          {"typ_p",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(
                 k, v, [self](float f) { self->model_config_.typ_p = f; });
           }},
          {"repeat_penalty",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.repeat_penalty = f;
             });
           }},
          {"mirostat_tau",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.mirostat_tau = f;
             });
           }},
          {"mirostat_eta",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.mirostat_eta = f;
             });
           }},
          {"max_tokens",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.max_tokens = static_cast<int>(f);
             });
           }},
          {"ngl",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.ngl = static_cast<int>(f);
             });
           }},
          {"ctx_len",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.ctx_len = static_cast<int>(f);
             });
           }},
          {"tp",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.tp = static_cast<int>(f);
             });
           }},
          {"seed",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.seed = static_cast<int>(f);
             });
           }},
          {"top_k",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.top_k = static_cast<int>(f);
             });
           }},
          {"repeat_last_n",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.repeat_last_n = static_cast<int>(f);
             });
           }},
          {"n_probs",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.n_probs = static_cast<int>(f);
             });
           }},
          {"min_keep",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.min_keep = static_cast<int>(f);
             });
           }},
          {"stream",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateBooleanField(
                 k, v, [self](bool b) { self->model_config_.stream = b; });
           }},
          {"text_model",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateBooleanField(
                 k, v, [self](bool b) { self->model_config_.text_model = b; });
           }},
          {"mirostat",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateBooleanField(
                 k, v, [self](bool b) { self->model_config_.mirostat = b; });
           }},
          {"penalize_nl",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateBooleanField(
                 k, v, [self](bool b) { self->model_config_.penalize_nl = b; });
           }},
          {"ignore_eos",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateBooleanField(
                 k, v, [self](bool b) { self->model_config_.ignore_eos = b; });
           }},
          {"created",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.created = static_cast<std::size_t>(f);
             });
           }},
      };

  if (auto it = updaters.find(key); it != updaters.end()) {
    it->second(this, key, value);
    LogUpdate(key, value);
  } else {
    CLI_LOG("Warning: Unknown configuration key '" << key << "' ignored.");
  }
}

void ModelUpdCmd::UpdateVectorField(const std::string& key,
                                    const std::string& value) {
  std::vector<std::string> tokens;
  std::istringstream iss(value);
  std::string token;
  while (std::getline(iss, token, ',')) {
    tokens.push_back(token);
  }
  model_config_.stop = tokens;
}

void ModelUpdCmd::UpdateNumericField(const std::string& key,
                                     const std::string& value,
                                     std::function<void(float)> setter) {
  try {
    float numericValue = std::stof(value);
    setter(numericValue);
  } catch (const std::exception& e) {
    CLI_LOG("Failed to parse numeric value for " << key << ": " << e.what());
  }
}

void ModelUpdCmd::UpdateBooleanField(const std::string& key,
                                     const std::string& value,
                                     std::function<void(bool)> setter) {
  bool boolValue = (value == "true" || value == "1");
  setter(boolValue);
}

void ModelUpdCmd::LogUpdate(const std::string& key, const std::string& value) {
  CLI_LOG("Updated " << key << " to: " << value);
}

}  // namespace commands