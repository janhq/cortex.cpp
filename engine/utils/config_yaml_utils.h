#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "yaml-cpp/yaml.h"

namespace config_yaml_utils {
struct CortexConfig {
  std::string logFolderPath;
  std::string logLlamaCppPath;
  std::string logTensorrtLLMPath;
  std::string logOnnxPath;
  std::string dataFolderPath;

  int maxLogLines;
  std::string apiServerHost;
  std::string apiServerPort;
  uint64_t checkedForUpdateAt;
  uint64_t checkedForLlamacppUpdateAt;
  std::string latestRelease;

  std::string latestLlamacppRelease;
  std::string huggingFaceToken;
  /**
   * Github's API requires a user-agent string.
   */
  std::string gitHubUserAgent;
  std::string gitHubToken;
  std::string llamacppVariant;
  std::string llamacppVersion;

  // TODO: we are adding too many things into cortexrc
  bool enableCors;
  std::vector<std::string> allowedOrigins;
  std::string proxyUrl;
  bool verifyProxySsl;
};

const std::string kDefaultHost{"127.0.0.1"};
const std::string kDefaultPort{"39281"};
const int kDefaultMaxLines{100000};
constexpr const uint64_t kDefaultCheckedForUpdateAt = 0u;
constexpr const uint64_t kDefaultCheckedForLlamacppUpdateAt = 0u;
constexpr const auto kDefaultLatestRelease = "default_version";
constexpr const auto kDefaultLatestLlamacppRelease = "";
constexpr const auto kDefaultCorsEnabled = true;
const std::vector<std::string> kDefaultEnabledOrigins{
    "http://localhost:39281", "http://127.0.0.1:39281", "http://0.0.0.0:39281"};

inline cpp::result<void, std::string> DumpYamlConfig(const CortexConfig& config,
                                                     const std::string& path) {
  std::filesystem::path config_file_path{path};

  try {
    std::ofstream out_file(config_file_path);
    if (!out_file) {
      throw std::runtime_error("Failed to open output file.");
    }
    YAML::Node node;
    node["logFolderPath"] = config.logFolderPath;
    node["logLlamaCppPath"] = config.logLlamaCppPath;
    node["logTensorrtLLMPath"] = config.logTensorrtLLMPath;
    node["logOnnxPath"] = config.logOnnxPath;
    node["dataFolderPath"] = config.dataFolderPath;
    node["maxLogLines"] = config.maxLogLines;
    node["apiServerHost"] = config.apiServerHost;
    node["apiServerPort"] = config.apiServerPort;
    node["checkedForUpdateAt"] = config.checkedForUpdateAt;
    node["checkedForLlamacppUpdateAt"] = config.checkedForLlamacppUpdateAt;
    node["latestRelease"] = config.latestRelease;
    node["latestLlamacppRelease"] = config.latestLlamacppRelease;
    node["huggingFaceToken"] = config.huggingFaceToken;
    node["gitHubUserAgent"] = config.gitHubUserAgent;
    node["gitHubToken"] = config.gitHubToken;
    node["llamacppVariant"] = config.llamacppVariant;
    node["llamacppVersion"] = config.llamacppVersion;
    node["enableCors"] = config.enableCors;
    node["allowedOrigins"] = config.allowedOrigins;
    node["proxyUrl"] = config.proxyUrl;
    node["verifyProxySsl"] = config.verifyProxySsl;

    out_file << node;
    out_file.close();
    return {};
  } catch (const std::exception& e) {
    CTL_ERR("Error writing to file: " << e.what());
    return cpp::fail("Error writing to file: " + std::string(e.what()));
  }
}

inline CortexConfig FromYaml(const std::string& path,
                             const CortexConfig& default_cfg) {
  std::filesystem::path config_file_path{path};
  if (!std::filesystem::exists(config_file_path)) {
    throw std::runtime_error("File not found: " + path);
  }

  try {
    auto node = YAML::LoadFile(config_file_path.string());
    bool should_update_config =
        (!node["logFolderPath"] || !node["dataFolderPath"] ||
         !node["maxLogLines"] || !node["apiServerHost"] ||
         !node["apiServerPort"] || !node["checkedForUpdateAt"] ||
         !node["checkedForLlamacppUpdateAt"] || !node["latestRelease"] ||
         !node["latestLlamacppRelease"] || !node["logLlamaCppPath"] ||
         !node["logOnnxPath"] || !node["logTensorrtLLMPath"] ||
         !node["huggingFaceToken"] || !node["gitHubUserAgent"] ||
         !node["gitHubToken"] || !node["llamacppVariant"] ||
         !node["llamacppVersion"] || !node["enableCors"] ||
         !node["allowedOrigins"] || !node["proxyUrl"] ||
         !node["verifyProxySsl"]);

    CortexConfig config = {
        .logFolderPath = node["logFolderPath"]
                             ? node["logFolderPath"].as<std::string>()
                             : default_cfg.logFolderPath,
        .logLlamaCppPath = node["logLlamaCppPath"]
                               ? node["logLlamaCppPath"].as<std::string>()
                               : default_cfg.logLlamaCppPath,
        .logTensorrtLLMPath = node["logTensorrtLLMPath"]
                                  ? node["logTensorrtLLMPath"].as<std::string>()
                                  : default_cfg.logTensorrtLLMPath,
        .logOnnxPath = node["logOnnxPath"]
                           ? node["logOnnxPath"].as<std::string>()
                           : default_cfg.logOnnxPath,
        .dataFolderPath = node["dataFolderPath"]
                              ? node["dataFolderPath"].as<std::string>()
                              : default_cfg.dataFolderPath,
        .maxLogLines = node["maxLogLines"] ? node["maxLogLines"].as<int>()
                                           : default_cfg.maxLogLines,
        .apiServerHost = node["apiServerHost"]
                             ? node["apiServerHost"].as<std::string>()
                             : default_cfg.apiServerHost,
        .apiServerPort = node["apiServerPort"]
                             ? node["apiServerPort"].as<std::string>()
                             : default_cfg.apiServerPort,
        .checkedForUpdateAt = node["checkedForUpdateAt"]
                                  ? node["checkedForUpdateAt"].as<uint64_t>()
                                  : default_cfg.checkedForUpdateAt,
        .checkedForLlamacppUpdateAt =
            node["checkedForLlamacppUpdateAt"]
                ? node["checkedForLlamacppUpdateAt"].as<uint64_t>()
                : default_cfg.checkedForLlamacppUpdateAt,
        .latestRelease = node["latestRelease"]
                             ? node["latestRelease"].as<std::string>()
                             : default_cfg.latestRelease,
        .latestLlamacppRelease =
            node["latestLlamacppRelease"]
                ? node["latestLlamacppRelease"].as<std::string>()
                : default_cfg.latestLlamacppRelease,
        .huggingFaceToken = node["huggingFaceToken"]
                                ? node["huggingFaceToken"].as<std::string>()
                                : default_cfg.huggingFaceToken,
        .gitHubUserAgent = node["gitHubUserAgent"]
                               ? node["gitHubUserAgent"].as<std::string>()
                               : default_cfg.gitHubUserAgent,
        .gitHubToken = node["gitHubToken"]
                           ? node["gitHubToken"].as<std::string>()
                           : default_cfg.gitHubToken,
        .llamacppVariant = node["llamacppVariant"]
                               ? node["llamacppVariant"].as<std::string>()
                               : default_cfg.llamacppVariant,
        .llamacppVersion = node["llamacppVersion"]
                               ? node["llamacppVersion"].as<std::string>()
                               : default_cfg.llamacppVersion,
        .enableCors = node["enableCors"] ? node["enableCors"].as<bool>()
                                         : default_cfg.enableCors,
        .allowedOrigins =
            node["allowedOrigins"]
                ? node["allowedOrigins"].as<std::vector<std::string>>()
                : default_cfg.allowedOrigins,
        .proxyUrl = node["proxyUrl"] ? node["proxyUrl"].as<std::string>()
                                     : default_cfg.proxyUrl,
        .verifyProxySsl = node["verifyProxySsl"]
                              ? node["verifyProxySsl"].as<bool>()
                              : default_cfg.verifyProxySsl,
    };
    if (should_update_config) {
      auto result = DumpYamlConfig(config, path);
      if (result.has_error()) {
        CTL_ERR("Failed to update config file: " << result.error());
      }
    }
    return config;
  } catch (const YAML::BadFile& e) {
    CTL_ERR("Failed to read file: " << e.what());
    throw;
  }
}
}  // namespace config_yaml_utils
