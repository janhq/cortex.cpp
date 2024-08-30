#pragma once
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "yaml-cpp/yaml.h"
#include "model_config.h"
namespace config {
class YamlHandler {
 private:
  YAML::Node yaml_node_;
  ModelConfig model_config_;
  void ReadYamlFile(const std::string& file_path);
  void ModelConfigFromYaml();
  void SplitPromptTemplate(ModelConfig& mc);

 public:
  // Method to read YAML file
  void Reset();

  const ModelConfig& GetModelConfig() const;

  void ModelConfigFromFile(const std::string& file_path);

  void UpdateModelConfig(ModelConfig new_model_config);
  // Method to write all attributes to a YAML file
  void WriteYamlFile(const std::string& file_path) const;
};
}