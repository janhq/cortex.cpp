#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace file_manager_utils {

inline std::filesystem::path GetContainerFolderPath(
    const std::string_view type) {
  const auto current_path{std::filesystem::current_path()};
  auto container_folder_path = std::filesystem::path{};

  if (type == "Model") {
    container_folder_path = current_path / "models";
  } else if (type == "Engine") {
    container_folder_path = current_path / "engines";
  } else {
    container_folder_path = current_path / "misc";
  }

  if (!std::filesystem::exists(container_folder_path)) {
    LOG_INFO << "Creating folder: " << container_folder_path.string() << "\n";
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
    default:
      return "UNKNOWN";
  }
}

}  // namespace file_manager_utils