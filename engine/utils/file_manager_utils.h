#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include "logging_utils.h"
#include "services/download_service.h"
#include "utils/config_yaml_utils.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace file_manager_utils {
constexpr std::string_view kCortexConfigurationFileName = ".cortexrc";
constexpr std::string_view kDefaultConfigurationPath = "user_home";
constexpr std::string_view kProdVariant = "prod";
constexpr std::string_view kBetaVariant = "beta";
constexpr std::string_view kNightlyVariant = "nightly";
constexpr char kLogsLlamacppBaseName[] = "./logs/cortex.log";
constexpr char kLogsTensorrtllmBaseName[] = "./logs/cortex.log";
constexpr char kLogsOnnxBaseName[] = "./logs/cortex.log";

inline std::filesystem::path GetExecutableFolderContainerPath() {
#if defined(__APPLE__) && defined(__MACH__)
  char buffer[1024];
  uint32_t size = sizeof(buffer);

  if (_NSGetExecutablePath(buffer, &size) == 0) {
    CTL_INF("Executable path: " << buffer);
    return std::filesystem::path{buffer}.parent_path();
  } else {
    CTL_ERR("Failed to get executable path");
    return std::filesystem::current_path();
  }
#elif defined(__linux__)
  char buffer[1024];
  ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (len != -1) {
    buffer[len] = '\0';
    CTL_INF("Executable path: " << buffer);
    return std::filesystem::path{buffer}.parent_path();
  } else {
    CTL_ERR("Failed to get executable path");
    return std::filesystem::current_path();
  }
#elif defined(_WIN32)
  char buffer[MAX_PATH];
  GetModuleFileNameA(NULL, buffer, MAX_PATH);
  CTL_INF("Executable path: " << buffer);
  return std::filesystem::path{buffer}.parent_path();
#else
  LOG_ERROR << "Unsupported platform!";
  return std::filesystem::current_path();
#endif
}

inline std::filesystem::path GetHomeDirectoryPath() {
#ifdef _WIN32
  const char* homeDir = std::getenv("USERPROFILE");
  if (!homeDir) {
    // Fallback if USERPROFILE is not set
    const char* homeDrive = std::getenv("HOMEDRIVE");
    const char* homePath = std::getenv("HOMEPATH");
    if (homeDrive && homePath) {
      return std::filesystem::path(homeDrive) / std::filesystem::path(homePath);
    } else {
      throw std::runtime_error("Cannot determine the home directory");
    }
  }
#else
  const char* homeDir = std::getenv("HOME");
  if (!homeDir) {
    throw std::runtime_error("Cannot determine the home directory");
  }
#endif
  return std::filesystem::path(homeDir);
}

inline std::filesystem::path GetConfigurationPath() {
#ifndef CORTEX_CONFIG_FILE_PATH
#define CORTEX_CONFIG_FILE_PATH kDefaultConfigurationPath
#endif

#ifndef CORTEX_VARIANT
#define CORTEX_VARIANT kProdVariant
#endif
  std::string config_file_path{CORTEX_CONFIG_FILE_PATH};

  if (config_file_path != kDefaultConfigurationPath) {
    CTL_INF("Config file path: " + config_file_path);
    return std::filesystem::path(config_file_path);
  }

  std::string variant{CORTEX_VARIANT};
  std::string env_postfix{""};
  if (variant == kBetaVariant) {
    env_postfix.append("-").append(kBetaVariant);
  } else if (variant == kNightlyVariant) {
    env_postfix.append("-").append(kNightlyVariant);
  }

  std::string config_file_name{kCortexConfigurationFileName};
  config_file_name.append(env_postfix);
  CTL_INF("Config file name: " + config_file_name);

  auto home_path = GetHomeDirectoryPath();
  auto configuration_path = home_path / config_file_name;
  return configuration_path;
}

inline std::string GetDefaultDataFolderName() {
#ifndef CORTEX_VARIANT
#define CORTEX_VARIANT "prod"
#endif
  std::string default_data_folder_name{config_yaml_utils::kCortexFolderName};
  std::string variant{CORTEX_VARIANT};
  std::string env_postfix{""};
  if (variant == kBetaVariant) {
    env_postfix.append("-").append(kBetaVariant);
  } else if (variant == kNightlyVariant) {
    env_postfix.append("-").append(kNightlyVariant);
  }
  default_data_folder_name.append(env_postfix);
  return default_data_folder_name;
}

inline void CreateConfigFileIfNotExist() {
  auto config_path = GetConfigurationPath();
  if (std::filesystem::exists(config_path)) {
    // already exists
    return;
  }

  auto default_data_folder_name = GetDefaultDataFolderName();

  CLI_LOG("Config file not found. Creating one at " + config_path.string());
  auto defaultDataFolderPath =
      file_manager_utils::GetHomeDirectoryPath() / default_data_folder_name;
  CTL_INF("Default data folder path: " + defaultDataFolderPath.string());

  auto config = config_yaml_utils::CortexConfig{
      .logFolderPath = defaultDataFolderPath.string(),
      .logLlamaCppPath = kLogsLlamacppBaseName,
      .logTensorrtLLMPath = kLogsTensorrtllmBaseName,
      .logOnnxPath = kLogsOnnxBaseName,
      .dataFolderPath = defaultDataFolderPath.string(),
      .maxLogLines = config_yaml_utils::kDefaultMaxLines,
      .apiServerHost = config_yaml_utils::kDefaultHost,
      .apiServerPort = config_yaml_utils::kDefaultPort,
  };
  DumpYamlConfig(config, config_path.string());
}

inline config_yaml_utils::CortexConfig GetCortexConfig() {
  auto config_path = GetConfigurationPath();
  auto default_data_folder_name = GetDefaultDataFolderName();
  auto default_data_folder_path =
      file_manager_utils::GetHomeDirectoryPath() / default_data_folder_name;
  auto default_cfg = config_yaml_utils::CortexConfig{
      .logFolderPath = default_data_folder_path.string(),
      .logLlamaCppPath = kLogsLlamacppBaseName,
      .logTensorrtLLMPath = kLogsTensorrtllmBaseName,
      .logOnnxPath = kLogsOnnxBaseName,
      .dataFolderPath = default_data_folder_path.string(),
      .maxLogLines = config_yaml_utils::kDefaultMaxLines,
      .apiServerHost = config_yaml_utils::kDefaultHost,
      .apiServerPort = config_yaml_utils::kDefaultPort,
      .checkedForUpdateAt = config_yaml_utils::kDefaultCheckedForUpdateAt,
      .latestRelease = config_yaml_utils::kDefaultLatestRelease,
  };

  return config_yaml_utils::FromYaml(config_path.string(), default_cfg);
}

inline std::filesystem::path GetCortexDataPath() {
  CreateConfigFileIfNotExist();
  auto config = GetCortexConfig();
  std::filesystem::path data_folder_path;
  if (!config.dataFolderPath.empty()) {
    data_folder_path = std::filesystem::path(config.dataFolderPath);
  } else {
    auto home_path = GetHomeDirectoryPath();
    data_folder_path = home_path / config_yaml_utils::kCortexFolderName;
  }

  if (!std::filesystem::exists(data_folder_path)) {
    CTL_INF("Cortex home folder not found. Create one: " +
            data_folder_path.string());
    std::filesystem::create_directory(data_folder_path);
  }
  return data_folder_path;
}

inline std::filesystem::path GetCortexLogPath() {
  // TODO: We will need to support user to move the data folder to other place.
  // TODO: get the variant of cortex. As discussed, we will have: prod, beta, nightly
  // currently we will store cortex data at ~/cortexcpp
  auto config = GetCortexConfig();
  std::filesystem::path log_folder_path;
  if (!config.logFolderPath.empty()) {
    log_folder_path = std::filesystem::path(config.logFolderPath);
  } else {
    auto home_path = GetHomeDirectoryPath();
    log_folder_path = home_path / config_yaml_utils::kCortexFolderName;
  }

  if (!std::filesystem::exists(log_folder_path)) {
    CTL_INF("Cortex log folder not found. Create one: " +
            log_folder_path.string());
    std::filesystem::create_directory(log_folder_path);
  }
  return log_folder_path;
}

inline void CreateDirectoryRecursively(const std::string& path) {
  // Create the directories if they don't exist
  if (std::filesystem::create_directories(path)) {
    CTL_INF(path + " successfully created!");
  } else {
    CTL_INF(path + " already exist!");
  }
}

inline std::filesystem::path GetModelsContainerPath() {
  CreateConfigFileIfNotExist();
  auto cortex_path = GetCortexDataPath();
  auto models_container_path = cortex_path / "models";

  if (!std::filesystem::exists(models_container_path)) {
    CTL_INF("Model container folder not found. Create one: "
            << models_container_path.string());
    std::filesystem::create_directories(models_container_path);
  }

  return models_container_path;
}

inline std::filesystem::path GetEnginesContainerPath() {
  auto cortex_path = GetCortexDataPath();
  auto engines_container_path = cortex_path / "engines";

  if (!std::filesystem::exists(engines_container_path)) {
    CTL_INF("Engine container folder not found. Create one: "
            << engines_container_path.string());
    std::filesystem::create_directory(engines_container_path);
  }

  return engines_container_path;
}

inline std::filesystem::path GetContainerFolderPath(
    const std::string_view type) {
  std::filesystem::path container_folder_path;

  if (type == "Model") {
    container_folder_path = GetModelsContainerPath();
  } else if (type == "Engine") {
    container_folder_path = GetEnginesContainerPath();
  } else if (type == "CudaToolkit") {
    container_folder_path =
        std::filesystem::temp_directory_path() / "cuda-dependencies";
  } else if (type == "Cortex") {
    container_folder_path = std::filesystem::temp_directory_path() / "cortex";
  } else {
    container_folder_path = std::filesystem::temp_directory_path() / "misc";
  }

  if (!std::filesystem::exists(container_folder_path)) {
    CTL_INF("Creating folder: " << container_folder_path.string() << "\n");
    std::filesystem::create_directories(container_folder_path);
  }

  return container_folder_path;
}

inline std::string DownloadTypeToString(DownloadType type) {
  switch (type) {
    case DownloadType::Model:
      return "Model";
    case DownloadType::Engine:
      return "Engine";
    case DownloadType::Miscellaneous:
      return "Misc";
    case DownloadType::CudaToolkit:
      return "CudaToolkit";
    case DownloadType::Cortex:
      return "Cortex";
    default:
      return "UNKNOWN";
  }
}

}  // namespace file_manager_utils
