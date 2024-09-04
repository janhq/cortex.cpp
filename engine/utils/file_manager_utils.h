#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include "logging_utils.h"
#include "services/download_service.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace file_manager_utils {
constexpr std::string_view kCortexConfigurationFileName = ".cortexrc";

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
  auto home_path = GetHomeDirectoryPath();
  auto configuration_path = home_path / kCortexConfigurationFileName;
  return configuration_path;
}

inline std::filesystem::path GetCortexPath() {
  // TODO: We will need to support user to move the data folder to other place.
  // TODO: get the variant of cortex. As discussed, we will have: prod, beta, nightly
  // currently we will store cortex data at ~/.cortex

  auto home_path = GetHomeDirectoryPath();
  auto cortex_path = home_path / ".cortex";
  if (!std::filesystem::exists(cortex_path)) {
    CTL_INF("Cortex home folder not found. Create one: " +
            cortex_path.string());
    std::filesystem::create_directory(cortex_path);
  }
  return cortex_path;
}

inline std::filesystem::path GetModelsContainerPath() {
  auto cortex_path = GetCortexPath();
  auto models_container_path = cortex_path / "models";

  if (!std::filesystem::exists(models_container_path)) {
    CTL_INF("Model container folder not found. Create one: "
            << models_container_path.string());
    std::filesystem::create_directory(models_container_path);
  }

  return models_container_path;
}

inline std::filesystem::path GetEnginesContainerPath() {
  auto cortex_path = GetCortexPath();
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
    default:
      return "UNKNOWN";
  }
}

}  // namespace file_manager_utils
