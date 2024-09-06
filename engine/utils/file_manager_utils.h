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
constexpr std::string_view kCortexConfigurationFileName = "cortexrc";
constexpr std::string_view kDefaultConfigurationPath = "user_home";

inline std::filesystem::path GetExecutableFolderContainerPath() {
#if defined(__APPLE__) && defined(__MACH__)
  char buffer[1024];
  uint32_t size = sizeof(buffer);

  if (_NSGetExecutablePath(buffer, &size) == 0) {
    LOG_INFO << "Executable path: " << buffer;
    return std::filesystem::path{buffer}.parent_path();
  } else {
    LOG_ERROR << "Failed to get executable path";
    return std::filesystem::current_path();
  }
#elif defined(__linux__)
  char buffer[1024];
  ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (len != -1) {
    buffer[len] = '\0';
    LOG_INFO << "Executable path: " << buffer;
    return std::filesystem::path{buffer}.parent_path();
  } else {
    LOG_ERROR << "Failed to get executable path";
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
  std::string config_file_path{CORTEX_CONFIG_FILE_PATH};

  if (config_file_path != kDefaultConfigurationPath) {
    return std::filesystem::path(config_file_path);
  }
  auto home_path = GetHomeDirectoryPath();
  auto configuration_path = home_path / kCortexConfigurationFileName;
  return configuration_path;
}

inline void CreateConfigFileIfNotExist() {
  auto config_path = file_manager_utils::GetConfigurationPath();
  if (std::filesystem::exists(config_path)) {
    // already exists
    return;
  }
  CLI_LOG("Config file not found. Creating one at " + config_path.string());
  auto defaultDataFolderPath =
      file_manager_utils::GetHomeDirectoryPath() / config_yaml_utils::kCortexFolderName;
  auto config = config_yaml_utils::CortexConfig{
      .dataFolderPath = defaultDataFolderPath.string(),
      .host = config_yaml_utils::kDefaultHost,
      .port = config_yaml_utils::kDefaultPort,
  };
  std::cout << "config: " << config.dataFolderPath << "\n";
  DumpYamlConfig(config, config_path.string());
}

inline config_yaml_utils::CortexConfig GetCortexConfig() {
  auto config_path = GetConfigurationPath();
  std::string variant = "";  // TODO: empty for now
  return config_yaml_utils::FromYaml(config_path.string(), variant);
}

inline std::filesystem::path GetCortexDataPath() {
  // TODO: We will need to support user to move the data folder to other place.
  // TODO: get the variant of cortex. As discussed, we will have: prod, beta, nightly
  // currently we will store cortex data at ~/cortexcpp
  auto config = GetCortexConfig();
  std::filesystem::path data_folder_path;
  if (!config.dataFolderPath.empty()) {
    data_folder_path =
        std::filesystem::path(config.dataFolderPath);
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

inline std::filesystem::path GetModelsContainerPath() {
  auto cortex_path = GetCortexDataPath();
  auto models_container_path = cortex_path / "models";

  if (!std::filesystem::exists(models_container_path)) {
    CTL_INF("Model container folder not found. Create one: "
            << models_container_path.string());
    std::filesystem::create_directory(models_container_path);
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
  const auto current_path{GetExecutableFolderContainerPath()};
  auto container_folder_path = std::filesystem::path{};

  if (type == "Model") {
    container_folder_path = GetModelsContainerPath();
  } else if (type == "Engine") {
    container_folder_path = GetEnginesContainerPath();
  } else if (type == "CudaToolkit") {
    container_folder_path = current_path;
  } else if (type == "Cortex") {
    container_folder_path = current_path / "cortex";
  } else {
    container_folder_path = current_path / "misc";
  }

  if (!std::filesystem::exists(container_folder_path)) {
    CTL_INF("Creating folder: " << container_folder_path.string() << "\n");
    std::filesystem::create_directory(container_folder_path);
  }

  return container_folder_path;
}

inline std::string downloadTypeToString(DownloadType type) {
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
