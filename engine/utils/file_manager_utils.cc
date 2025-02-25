#include "file_manager_utils.h"

#include "logging_utils.h"

#include "utils/engine_constants.h"
#include "utils/result.hpp"
#include "utils/widechar_conv.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#include <codecvt>
#include <locale>
#endif

namespace file_manager_utils {

std::filesystem::path GetExecutablePath() {
#if defined(__APPLE__) && defined(__MACH__)
  char buffer[1024];
  uint32_t size = sizeof(buffer);

  if (_NSGetExecutablePath(buffer, &size) == 0) {
    // CTL_DBG("Executable path: " << buffer);
    return std::filesystem::path{buffer};
  } else {
    CTL_ERR("Failed to get executable path");
    return std::filesystem::current_path();
  }
#elif defined(__linux__)
  char buffer[1024];
  ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (len != -1) {
    buffer[len] = '\0';
    // CTL_DBG("Executable path: " << buffer);
    return std::filesystem::path{buffer};
  } else {
    CTL_ERR("Failed to get executable path");
    return std::filesystem::current_path();
  }
#elif defined(_WIN32)
  wchar_t buffer[MAX_PATH];
  GetModuleFileNameW(NULL, buffer, MAX_PATH);
  // CTL_DBG("Executable path: " << buffer);
  return std::filesystem::path{buffer};
#else
  LOG_ERROR << "Unsupported platform!";
  return std::filesystem::current_path();
#endif
}

std::filesystem::path GetExecutableFolderContainerPath() {
  return GetExecutablePath().parent_path();
}

std::filesystem::path GetHomeDirectoryPath() {
#ifdef _WIN32
  const wchar_t* homeDir = _wgetenv(L"USERPROFILE");
  if (!homeDir) {
    // Fallback if USERPROFILE is not set
    const wchar_t* homeDrive = _wgetenv(L"HOMEDRIVE");
    const wchar_t* homePath = _wgetenv(L"HOMEPATH");
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

// Helper function to get XDG base directory, falling back to default if not set
std::filesystem::path GetXDGDirectory(const char* envVar,
                                      const char* defaultPath) {
  const char* envValue = getenv(envVar);
  if (envValue && *envValue) {  // Check if envValue is not NULL and not empty
    return std::filesystem::path(envValue);
  }
  return GetHomeDirectoryPath() / defaultPath;
}

std::filesystem::path GetConfigurationPath() {
#ifndef CORTEX_CONFIG_FILE_PATH
#define CORTEX_CONFIG_FILE_PATH kDefaultConfigurationPath
#endif

#ifndef CORTEX_VARIANT
#define CORTEX_VARIANT kProdVariant
#endif
  std::string config_file_path;
  if (cortex_config_file_path.empty()) {
    config_file_path = CORTEX_CONFIG_FILE_PATH;
  } else {
    config_file_path = cortex_config_file_path;
  }

  if (config_file_path != kDefaultConfigurationPath) {
// CTL_INF("Config file path: " + config_file_path);
#if defined(_WIN32)
    return std::filesystem::u8path(config_file_path);
#else
    return std::filesystem::path(config_file_path);
#endif
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
  // CTL_INF("Config file name: " + config_file_name);
#if defined(__linux__)
  auto config_base_path =
      GetXDGDirectory("XDG_CONFIG_HOME", ".config") / kCortexFolderName;
  auto configuration_path = config_base_path / config_file_name;
  if (!std::filesystem::exists(config_base_path)) {
    CTL_INF("Cortex config folder not found. Create one: " +
            config_base_path.string());
    std::filesystem::create_directory(config_base_path);
  }
#else
  auto home_path = GetHomeDirectoryPath();
  auto configuration_path = home_path / config_file_name;
#endif
  return configuration_path;
}

std::string GetDefaultDataFolderName() {
#ifndef CORTEX_VARIANT
#define CORTEX_VARIANT "prod"
#endif
  std::string default_data_folder_name{kCortexFolderName};
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

cpp::result<void, std::string> UpdateCortexConfig(
    const config_yaml_utils::CortexConfig& config) {
  auto config_path = GetConfigurationPath();
  if (!std::filesystem::exists(config_path)) {
    CTL_ERR("Config file not found: " << config_path.string());
    return cpp::fail("Config file not found: " + config_path.string());
  }

  return cyu::CortexConfigMgr::GetInstance().DumpYamlConfig(
      config, config_path.string());
}

config_yaml_utils::CortexConfig GetDefaultConfig() {
  auto config_path = GetConfigurationPath();
  auto default_data_folder_name = GetDefaultDataFolderName();
  auto default_data_folder_path =
      cortex_data_folder_path.empty()
#if defined(__linux__)
          ? file_manager_utils::GetXDGDirectory("XDG_DATA_HOME",
                                                ".local/share") /
                default_data_folder_name
#else
          ? file_manager_utils::GetHomeDirectoryPath() /
                default_data_folder_name
#endif
          : std::filesystem::path(cortex_data_folder_path);

  return config_yaml_utils::CortexConfig{
#if defined(_WIN32)
      .logFolderPath =
          cortex::wc::WstringToUtf8(default_data_folder_path.wstring()),
#else
      .logFolderPath = default_data_folder_path.string(),
#endif
      .logLlamaCppPath = kLogsLlamacppBaseName,
      .logTensorrtLLMPath = kLogsTensorrtllmBaseName,
      .logOnnxPath = kLogsOnnxBaseName,
#if defined(_WIN32)
      .dataFolderPath =
          cortex::wc::WstringToUtf8(default_data_folder_path.wstring()),
#else
      .dataFolderPath = default_data_folder_path.string(),
#endif
      .maxLogLines = config_yaml_utils::kDefaultMaxLines,
      .apiServerHost = config_yaml_utils::kDefaultHost,
      .apiServerPort = config_yaml_utils::kDefaultPort,
      .checkedForUpdateAt = config_yaml_utils::kDefaultCheckedForUpdateAt,
      .checkedForLlamacppUpdateAt =
          config_yaml_utils::kDefaultCheckedForLlamacppUpdateAt,
      .latestRelease = config_yaml_utils::kDefaultLatestRelease,
      .latestLlamacppRelease = config_yaml_utils::kDefaultLatestLlamacppRelease,
      .enableCors = config_yaml_utils::kDefaultCorsEnabled,
      .allowedOrigins = config_yaml_utils::kDefaultEnabledOrigins,
      .proxyUrl = "",
      .verifyProxySsl = true,
      .verifyProxyHostSsl = true,
      .proxyUsername = "",
      .proxyPassword = "",
      .noProxy = config_yaml_utils::kDefaultNoProxy,
      .verifyPeerSsl = true,
      .verifyHostSsl = true,

      .sslCertPath = "",
      .sslKeyPath = "",
      .supportedEngines = config_yaml_utils::kDefaultSupportedEngines,
      .checkedForSyncHubAt = 0u,
  };
}

cpp::result<void, std::string> CreateConfigFileIfNotExist() {
  auto config_path = GetConfigurationPath();
  if (std::filesystem::exists(config_path)) {
    // already exists, no need to create
    return {};
  }

  CLI_LOG("Config file not found. Creating one at " + config_path.string());
  auto config = GetDefaultConfig();
  CLI_LOG("Default data folder path: " + config.dataFolderPath);
  return cyu::CortexConfigMgr::GetInstance().DumpYamlConfig(
      config, config_path.string());
}

config_yaml_utils::CortexConfig GetCortexConfig() {
  auto config_path = GetConfigurationPath();

  auto default_cfg = GetDefaultConfig();
  return config_yaml_utils::CortexConfigMgr::GetInstance().FromYaml(
      config_path.string(), default_cfg);
}

std::filesystem::path GetCortexDataPath() {
  auto result = CreateConfigFileIfNotExist();
  if (result.has_error()) {
    CTL_ERR("Error creating config file: " << result.error());
    return std::filesystem::path{};
  }

  auto config = GetCortexConfig();
  std::filesystem::path data_folder_path;
  if (!config.dataFolderPath.empty()) {
#if defined(_WIN32)
    data_folder_path = std::filesystem::u8path(config.dataFolderPath);
#else
    data_folder_path = std::filesystem::path(config.dataFolderPath);
#endif
  } else {
#if defined(__linux__)
    auto data_base_path = GetXDGDirectory("XDG_DATA_HOME", ".local/share");
    data_folder_path = data_base_path / GetDefaultDataFolderName();
#else
    auto home_path = GetHomeDirectoryPath();
    data_folder_path = home_path / kCortexFolderName;
#endif
  }

  if (!std::filesystem::exists(data_folder_path)) {
    CLI_LOG("Cortex home folder not found. Create one: " +
            data_folder_path.string());
    std::filesystem::create_directory(data_folder_path);
  }
  return data_folder_path;
}

std::filesystem::path GetCortexLogPath() {
  // TODO: We will need to support user to move the data folder to other place.
  // TODO: get the variant of cortex. As discussed, we will have: prod, beta, nightly

  // currently we will store cortex data at ~/cortexcpp
  // On linux, we follow the xdg directory specification
  auto config = GetCortexConfig();
  std::filesystem::path log_folder_path;
  if (!config.logFolderPath.empty()) {
    log_folder_path = std::filesystem::path(config.logFolderPath);
  } else {
#if defined(__linux__)
    auto data_base_path = GetXDGDirectory("XDG_DATA_HOME", ".local/share");
    log_folder_path = data_base_path / GetDefaultDataFolderName() / "logs";
#else
    auto home_path = GetHomeDirectoryPath();
    log_folder_path = home_path / kCortexFolderName;
#endif
  }

  if (!std::filesystem::exists(log_folder_path)) {
    CTL_INF("Cortex log folder not found. Create one: " +
            log_folder_path.string());
    std::filesystem::create_directory(log_folder_path);
  }
  return log_folder_path;
}

void CreateDirectoryRecursively(const std::string& path) {
  // Create the directories if they don't exist
  if (std::filesystem::create_directories(path)) {
    CTL_INF(path + " successfully created!");
  } else {
    CTL_INF(path + " already exist!");
  }
}

std::filesystem::path GetModelsContainerPath() {
  auto result = CreateConfigFileIfNotExist();
  if (result.has_error()) {
    CTL_ERR("Error creating config file: " << result.error());
  }
  auto cortex_path = GetCortexDataPath();
  auto models_container_path = cortex_path / "models";

  if (!std::filesystem::exists(models_container_path)) {
    CTL_INF("Model container folder not found. Create one: "
            << models_container_path.string());
    std::filesystem::create_directories(models_container_path);
  }

  return models_container_path;
}

std::filesystem::path GetCudaToolkitPath(const std::string& engine,
                                         bool create_if_not_exist) {
  auto engine_path = getenv("ENGINE_PATH")
                         ? std::filesystem::path(getenv("ENGINE_PATH"))
                         : GetCortexDataPath();

  auto cuda_path = engine_path / "engines" / engine / "deps";
  if (create_if_not_exist && !std::filesystem::exists(cuda_path)) {
    std::filesystem::create_directories(cuda_path);
  }

  return cuda_path;
}

std::filesystem::path GetThreadsContainerPath() {
  auto cortex_path = GetCortexDataPath();
  return cortex_path / "threads";
}

std::filesystem::path GetEnginesContainerPath() {
  auto cortex_path = getenv("ENGINE_PATH")
                         ? std::filesystem::path(getenv("ENGINE_PATH"))
                         : GetCortexDataPath();
  auto engines_container_path = cortex_path / "engines";

  if (!std::filesystem::exists(engines_container_path)) {
    CTL_INF("Engine container folder not found. Create one: "
            << engines_container_path.string());
    std::filesystem::create_directory(engines_container_path);
  }

  return engines_container_path;
}

std::filesystem::path GetContainerFolderPath(const std::string_view type) {
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

std::string DownloadTypeToString(DownloadType type) {
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

std::filesystem::path ToRelativeCortexDataPath(
    const std::filesystem::path& path) {
  return Subtract(path, GetCortexDataPath());
}

std::filesystem::path ToAbsoluteCortexDataPath(
    const std::filesystem::path& path) {
  return GetAbsolutePath(GetCortexDataPath(), path);
}
}  // namespace file_manager_utils
