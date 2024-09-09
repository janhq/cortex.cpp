#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "yaml-cpp/yaml.h"

namespace config_yaml_utils {
struct CortexConfig {
  std::string dataFolderPath;
  std::string host;
  std::string port;
};

const std::string kCortexFolderName = "cortexcpp";
const std::string kDefaultHost{"127.0.0.1"};
const std::string kDefaultPort{"3928"};

inline void DumpYamlConfig(const CortexConfig& config,
                           const std::string& path) {
  std::filesystem::path config_file_path{path};

  try {
    std::ofstream out_file(config_file_path);
    if (!out_file) {
      throw std::runtime_error("Failed to open output file.");
    }
    YAML::Node node;
    node["dataFolderPath"] = config.dataFolderPath;
    node["host"] = config.host;
    node["port"] = config.port;

    out_file << node;
    out_file.close();
  } catch (const std::exception& e) {
    CTL_ERR("Error writing to file: " << e.what());
    throw;
  }
}

inline void CreateConfigFileIfNotExist() {
  auto config_path = file_manager_utils::GetConfigurationPath();
  if (std::filesystem::exists(config_path)) {
    // already exists
    return;
  }
  CLI_LOG("Config file not found. Creating one at " + config_path.string());
  auto defaultDataFolderPath =
      file_manager_utils::GetHomeDirectoryPath() / kCortexFolderName;
  auto config = CortexConfig{
      .dataFolderPath = defaultDataFolderPath.string(),
      .host = kDefaultHost,
      .port = kDefaultPort,
  };
  std::cout << "config: " << config.dataFolderPath << "\n";
  DumpYamlConfig(config, config_path.string());
}

inline CortexConfig FromYaml(const std::string& path,
                             const std::string& variant) {
  std::filesystem::path config_file_path{path};
  if (!std::filesystem::exists(config_file_path)) {
    throw std::runtime_error("File not found: " + path);
  }

  try {
    auto node = YAML::LoadFile(config_file_path.string());
    CortexConfig config = {
        .dataFolderPath = node["dataFolderPath"].as<std::string>(),
        .host = node["host"].as<std::string>(),
        .port = node["port"].as<std::string>(),
    };
    return config;
  } catch (const YAML::BadFile& e) {
    CTL_ERR("Failed to read file: " << e.what());
    throw;
  }
}

inline CortexConfig GetCortexConfig() {
  auto config_path = file_manager_utils::GetConfigurationPath();
  std::string variant = "";  // TODO: empty for now
  return FromYaml(config_path.string(), variant);
}
}  // namespace config_yaml_utils
