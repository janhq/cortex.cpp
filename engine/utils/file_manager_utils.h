#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include "common/download_task.h"
#include "utils/config_yaml_utils.h"

namespace file_manager_utils {
namespace cyu = config_yaml_utils;
constexpr std::string_view kCortexConfigurationFileName = ".cortexrc";
constexpr std::string_view kDefaultConfigurationPath = "user_home";
constexpr std::string_view kProdVariant = "prod";
constexpr std::string_view kBetaVariant = "beta";
constexpr std::string_view kNightlyVariant = "nightly";
constexpr char kLogsLlamacppBaseName[] = "./logs/cortex.log";
constexpr char kLogsOnnxBaseName[] = "./logs/cortex.log";

inline std::string cortex_config_file_path;

inline std::string cortex_data_folder_path;

std::filesystem::path GetExecutablePath();

std::filesystem::path GetExecutableFolderContainerPath();

std::filesystem::path GetHomeDirectoryPath();

std::filesystem::path GetConfigurationPath();

std::string GetDefaultDataFolderName();

cpp::result<void, std::string> UpdateCortexConfig(
    const config_yaml_utils::CortexConfig& config);

config_yaml_utils::CortexConfig GetDefaultConfig();

cpp::result<void, std::string> CreateConfigFileIfNotExist();

config_yaml_utils::CortexConfig GetCortexConfig();

std::filesystem::path GetCortexDataPath();

std::filesystem::path GetCortexLogPath();

void CreateDirectoryRecursively(const std::string& path);

std::filesystem::path GetModelsContainerPath();

std::filesystem::path GetCudaToolkitPath(const std::string& engine,
                                         bool create_if_not_exist = false);

std::filesystem::path GetEnginesContainerPath();

std::filesystem::path GetThreadsContainerPath();

std::filesystem::path GetContainerFolderPath(const std::string_view type);

std::string DownloadTypeToString(DownloadType type);

inline std::filesystem::path GetAbsolutePath(const std::filesystem::path& base,
                                             const std::filesystem::path& r) {
  if (r.is_absolute()) {
    return r;
  } else {
    return base / r;
  }
}

inline bool IsSubpath(const std::filesystem::path& base,
                      const std::filesystem::path& path) {
  if (base == path)
    return true;
  auto rel = std::filesystem::relative(path, base);
  return !rel.empty() && rel.native()[0] != '.';
}

inline std::filesystem::path Subtract(const std::filesystem::path& path,
                                      const std::filesystem::path& base) {
  if (IsSubpath(base, path)) {
    return path.lexically_relative(base);
  } else {
    return path;
  }
}

std::filesystem::path ToRelativeCortexDataPath(
    const std::filesystem::path& path);

std::filesystem::path ToAbsoluteCortexDataPath(
    const std::filesystem::path& path);

}  // namespace file_manager_utils
