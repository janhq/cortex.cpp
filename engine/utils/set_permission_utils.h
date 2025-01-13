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
[[nodiscard]] inline bool SetExecutePermission(const std::filesystem::path& filePath,
                                             bool ownerOnly = false) noexcept {
    try {
        std::filesystem::perms current_perms = std::filesystem::status(filePath).permissions();
        std::filesystem::perms new_perms;

        if (ownerOnly) {
            new_perms = current_perms | std::filesystem::perms::owner_exec;
            // Remove group and others execute permissions
            new_perms &= ~(std::filesystem::perms::group_exec | std::filesystem::perms::others_exec);
        } else {
            new_perms = current_perms | std::filesystem::perms::owner_exec |
                       std::filesystem::perms::group_exec |
                       std::filesystem::perms::others_exec;
        }

        std::filesystem::permissions(filePath, new_perms, 
                                   std::filesystem::perm_options::replace);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        CTL_ERR("Permission error for file " << filePath.string() 
                << ": " << e.what());
        return false;
    } catch (const std::exception& e) {
        CTL_ERR("Unexpected error for file " << filePath.string() 
                << ": " << e.what());
        return false;
    }
}

[[nodiscard]] inline std::vector<std::filesystem::path> SetExecutePermissionsRecursive(
    const std::filesystem::path& directoryPath,
    bool ownerOnly = false,
    bool skipDirectories = true) {
    std::vector<std::filesystem::path> modifiedFiles;
    modifiedFiles.reserve(100);  // Reserve space to prevent frequent reallocations

    try {
        const auto options = std::filesystem::directory_options::skip_permission_denied |
                           std::filesystem::directory_options::follow_directory_symlink;
        
        for (const auto& entry : 
             std::filesystem::recursive_directory_iterator(directoryPath, options)) {
            if (skipDirectories && entry.is_directory()) {
                continue;
            }

            if (entry.is_regular_file()) {
                if (SetExecutePermission(entry.path(), ownerOnly)) {
                    modifiedFiles.push_back(entry.path());
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        CTL_ERR("Filesystem error: " << e.what());
    }

    return modifiedFiles;
}

}  // namespace set_permission_utils