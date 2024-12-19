#pragma once

#include <filesystem>
#include <iostream>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif
#include "utils/logging_utils.h"
namespace set_permission_utils {
// Cross-platform method to set execute permission for a single file
inline bool SetExecutePermission(const std::filesystem::path& filePath,
                                 bool ownerOnly = false) {
  std::error_code ec;

#ifdef _WIN32
  // Windows execution permission handling
  std::filesystem::path exePath = filePath;

  // Add .exe extension if no extension exists
  if (exePath.extension().empty()) {
    exePath += ".exe";
    std::filesystem::rename(filePath, exePath);
  }

  // Clear read-only attribute
  DWORD fileAttributes = GetFileAttributes(exePath.c_str());
  if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
    CTL_ERROR("Error accessing file: " << GetLastError());
    return false;
  }

  fileAttributes &= ~FILE_ATTRIBUTE_READONLY;

  if (!SetFileAttributes(exePath.c_str(), fileAttributes)) {
    CTL_ERROR("Error setting file attributes: " << GetLastError());
    return false;
  }

#else
  // POSIX systems (Linux, macOS)
  struct stat st;
  if (stat(filePath.c_str(), &st) != 0) {
    CTL_ERR("Error getting file stats: " << strerror(errno));
    return false;
  }

  // Set execute permissions based on ownerOnly flag
  mode_t newMode;
  if (ownerOnly) {
    // Only owner can execute
    newMode = (st.st_mode & ~(S_IXGRP | S_IXOTH)) | S_IXUSR;
  } else {
    // Everyone can execute
    newMode = st.st_mode | S_IXUSR |  // Owner execute
              S_IXGRP |               // Group execute
              S_IXOTH;                // Others execute
  }

  if (chmod(filePath.c_str(), newMode) != 0) {
    CTL_ERR("Error setting execute permissions: " << strerror(errno));
    return false;
  }
#endif

  return true;
}
inline std::vector<std::filesystem::path> SetExecutePermissionsRecursive(
    const std::filesystem::path& directoryPath, bool ownerOnly = false,
    bool skipDirectories = true) {
  std::vector<std::filesystem::path> modifiedFiles;

  try {
    // Iterate through all files and subdirectories
    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(directoryPath)) {
      // Skip directories if specified
      if (skipDirectories && entry.is_directory()) {
        continue;
      }

      // Only process files
      if (entry.is_regular_file()) {
        try {
          if (SetExecutePermission(entry.path(), ownerOnly)) {
            modifiedFiles.push_back(entry.path());
          }
        } catch (const std::exception& e) {
          CTL_ERR("Error processing file " + entry.path().string() + ": " +
                  e.what());
        }
      }
    }
  } catch (const std::filesystem::filesystem_error& e) {
    CTL_ERR("Filesystem error: " << e.what());
  }

  return modifiedFiles;
}

}  // namespace set_permission_utils