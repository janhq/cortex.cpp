#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "config/model_config.h"
#include "config/yaml_config.h"
#include "database/models.h"
namespace commands {
class ModelUpdCmd {
 public:
  ModelUpdCmd(std::string model_handle);
  void Exec(const std::unordered_map<std::string, std::string>& options);

 private:
  std::string model_handle_;
  config::ModelConfig model_config_;
  config::YamlHandler yaml_handler_;
  cortex::db::Models model_list_utils_;

  void UpdateConfig(const std::string& key, const std::string& value);
  void UpdateVectorField(const std::string& key, const std::string& value);
  void UpdateNumericField(const std::string& key, const std::string& value,
                          std::function<void(float)> setter);
  void UpdateBooleanField(const std::string& key, const std::string& value,
                          std::function<void(bool)> setter);
  void LogUpdate(const std::string& key, const std::string& value);
};
}  // namespace commands