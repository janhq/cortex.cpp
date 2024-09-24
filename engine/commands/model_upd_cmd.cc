#include "model_upd_cmd.h"

#include "utils/logging_utils.h"

namespace commands {

ModelUpdCmd::ModelUpdCmd(std::string model_handle)
    : model_handle_(std::move(model_handle)) {}

void ModelUpdCmd::Exec(
    const std::unordered_map<std::string, std::string>& options) {
  try {
    auto model_entry = model_list_utils_.GetModelInfo(model_handle_);
    yaml_handler_.ModelConfigFromFile(model_entry.path_to_model_yaml);
    model_config_ = yaml_handler_.GetModelConfig();

    for (const auto& [key, value] : options) {
      if (!value.empty()) {
        UpdateConfig(key, value);
      }
    }

    yaml_handler_.UpdateModelConfig(model_config_);
    yaml_handler_.WriteYamlFile(model_entry.path_to_model_yaml);
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
          {"stop", &ModelUpdCmd::UpdateVectorField},
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
          {"max_tokens",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateNumericField(k, v, [self](float f) {
               self->model_config_.max_tokens = static_cast<int>(f);
             });
           }},
          {"stream",
           [](ModelUpdCmd* self, const std::string& k, const std::string& v) {
             self->UpdateBooleanField(
                 k, v, [self](bool b) { self->model_config_.stream = b; });
           }},
          // Add more fields here...
      };

  if (auto it = updaters.find(key); it != updaters.end()) {
    it->second(this, key, value);
    LogUpdate(key, value);
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