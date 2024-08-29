#pragma once
#include "logging_utils.h"
#include <filesystem>
#include <string>
#include <string_view>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace file_manager_utils {

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
  // TODO: haven't tested
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
  // TODO: haven't tested
  char buffer[MAX_PATH];
  GetModuleFileNameA(NULL, buffer, MAX_PATH);
  CTLOG_INFO("Executable path: " << buffer);
  return std::filesystem::path{buffer}.parent_path();
#else
  LOG_ERROR << "Unsupported platform!";
  return std::filesystem::current_path();
#endif
}

inline std::filesystem::path GetContainerFolderPath(
    const std::string_view type) {
  const auto current_path{GetExecutableFolderContainerPath()};
  auto container_folder_path = std::filesystem::path{};

  if (type == "Model") {
    container_folder_path = current_path / "models";
  } else if (type == "Engine") {
    container_folder_path = current_path / "engines";
  } else if (type == "CudaToolkit") {
    container_folder_path = current_path;
  } else {
    container_folder_path = current_path / "misc";
  }

  if (!std::filesystem::exists(container_folder_path)) {
    CTLOG_INFO("Creating folder: " << container_folder_path.string() << "\n");
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