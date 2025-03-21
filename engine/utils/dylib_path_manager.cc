#include "dylib_path_manager.h"
#include "utils/logging_utils.h"

namespace cortex {

cpp::result<void, std::string> DylibPathManager::RegisterPath(
    const std::string& key, std::vector<std::filesystem::path> paths) {
#if defined(_WIN32) || defined(_WIN64)
  std::vector<DylibPath> dylib_paths;
  for (const auto& path : paths) {
    if (!std::filesystem::exists(path)) {
      return cpp::fail("Path does not exist: " + path.string());
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide_path = converter.from_bytes(path.string());

    auto cookie = AddDllDirectory(wide_path.c_str());
    if (cookie == nullptr) {
      CTL_ERR("Failed to added DLL directory: " << path.string());

      // Clean up any paths we've already added
      for (auto& dylib_path : dylib_paths) {
        CTL_DBG("Cleaning DLL path: " + dylib_path.path.string());
        RemoveDllDirectory(dylib_path.cookie);
      }
      return cpp::fail("Failed to add DLL directory: " + path.string());
    } else {
      CTL_INF("Added DLL directory: " << path.string());
    }

    dylib_paths.push_back({path, cookie});
  }
  dylib_map_[key] = std::move(dylib_paths);

#elif defined(__linux__)
  // For Linux, we need to modify LD_LIBRARY_PATH
  std::vector<DylibPath> dylib_paths;
  std::stringstream new_path;
  bool first = true;

  // First verify all paths exist
  for (const auto& path : paths) {
    if (!std::filesystem::exists(path)) {
      return cpp::fail("Path does not exist: " + path.string());
    }
  }

  // Get current LD_LIBRARY_PATH
  const char* current_path = getenv(kLdLibraryPath);
  std::string current_paths = current_path ? current_path : "";
  CTL_DBG("Current paths: " << current_paths);

  // Add new paths
  for (const auto& path : paths) {
    if (!first) {
      new_path << ":";
    }
    new_path << path.string();
    dylib_paths.push_back({path});
    first = false;
  }

  // Append existing paths if they exist
  if (!current_paths.empty()) {
    new_path << ":" << current_paths;
  }
  CTL_DBG("New paths: " << new_path.str());
  // Set the new LD_LIBRARY_PATH
  if (setenv(kLdLibraryPath, new_path.str().c_str(), 1) != 0) {
    CTL_ERR("Failed to set path!!!");
    return cpp::fail("Failed to set " + std::string(kLdLibraryPath));
  }

  CTL_DBG("After set path: " << getenv(kLdLibraryPath));

  dylib_map_[key] = std::move(dylib_paths);
#endif

  return {};
}

cpp::result<void, std::string> DylibPathManager::Unregister(
    const std::string& key) {
  auto it = dylib_map_.find(key);
  if (it == dylib_map_.end()) {
    return cpp::fail("Key not found: " + key);
  }

#if defined(_WIN32) || defined(_WIN64)
  // For Windows, remove each DLL directory
  for (auto& dylib_path : it->second) {
    if (!RemoveDllDirectory(dylib_path.cookie)) {
      return cpp::fail("Failed to remove DLL directory: " +
                       dylib_path.path.string());
    }
  }

#elif defined(__linux__)
  // For Linux, we need to rebuild LD_LIBRARY_PATH without the removed paths
  const char* current_path = getenv(kLdLibraryPath);
  if (current_path) {
    std::string paths = current_path;
    for (const auto& dylib_path : it->second) {
      std::string path_str = dylib_path.path.string();
      size_t pos = paths.find(path_str);
      if (pos != std::string::npos) {
        // Remove the path and the following colon (or preceding colon if it's at the end)
        if (pos > 0 && paths[pos - 1] == ':') {
          paths.erase(pos - 1, path_str.length() + 1);
        } else if (pos + path_str.length() < paths.length() &&
                   paths[pos + path_str.length()] == ':') {
          paths.erase(pos, path_str.length() + 1);
        } else {
          paths.erase(pos, path_str.length());
        }
      }
    }

    if (setenv(kLdLibraryPath, paths.c_str(), 1) != 0) {
      return cpp::fail("Failed to update " + std::string(kLdLibraryPath));
    }
  }
#endif

  dylib_map_.erase(it);
  return {};
}
}  // namespace cortex
