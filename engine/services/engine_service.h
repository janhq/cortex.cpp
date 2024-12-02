#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "common/engine_servicei.h"
#include "cortex-common/EngineI.h"
#include "cortex-common/cortexpythoni.h"
#include "services/download_service.h"
#include "utils/cpuid/cpu_info.h"
#include "utils/dylib.h"
#include "utils/engine_constants.h"
#include "utils/github_release_utils.h"
#include "utils/result.hpp"
#include "utils/system_info_utils.h"

struct EngineUpdateResult {
  std::string engine;
  std::string variant;
  std::string from;
  std::string to;

  Json::Value ToJson() const {
    Json::Value root;
    root["engine"] = engine;
    root["variant"] = variant;
    root["from"] = from;
    root["to"] = to;
    return root;
  }
};

namespace system_info_utils {
struct SystemInfo;
}

using EngineV = std::variant<EngineI*, CortexPythonEngineI*>;

class EngineService : public EngineServiceI {
 private:
  using EngineRelease = github_release_utils::GitHubRelease;
  using EngineVariant = github_release_utils::GitHubAsset;

  struct EngineInfo {
    std::unique_ptr<cortex_cpp::dylib> dl;
    EngineV engine;
#if defined(_WIN32)
    DLL_DIRECTORY_COOKIE cookie;
    DLL_DIRECTORY_COOKIE cuda_cookie;
#endif
  };

  std::unordered_map<std::string, EngineInfo> engines_{};

 public:
  const std::vector<std::string_view> kSupportEngines = {
      kLlamaEngine, kOnnxEngine, kTrtLlmEngine};

  explicit EngineService(std::shared_ptr<DownloadService> download_service)
      : download_service_{download_service},
        hw_inf_{.sys_inf = system_info_utils::GetSystemInfo(),
                .cuda_driver_version =
                    system_info_utils::GetDriverAndCudaVersion().second} {}

  std::vector<EngineInfo> GetEngineInfoList() const;

  /**
   * Check if an engines is ready (have at least one variant installed)
   */
  cpp::result<bool, std::string> IsEngineReady(const std::string& engine) const;

  /**
   * Handling install engine variant.
   *
   * If no version provided, choose `latest`.
   * If no variant provided, automatically pick the best variant.
   */
  cpp::result<void, std::string> InstallEngineAsync(
      const std::string& engine, const std::string& version,
      const std::optional<std::string> variant_name);

  cpp::result<bool, std::string> UninstallEngineVariant(
      const std::string& engine, const std::optional<std::string> version,
      const std::optional<std::string> variant);

  cpp::result<std::vector<EngineRelease>, std::string> GetEngineReleases(
      const std::string& engine) const;

  cpp::result<std::vector<EngineVariant>, std::string> GetEngineVariants(
      const std::string& engine, const std::string& version) const;

  cpp::result<DefaultEngineVariant, std::string> SetDefaultEngineVariant(
      const std::string& engine, const std::string& version,
      const std::string& variant);

  cpp::result<DefaultEngineVariant, std::string> GetDefaultEngineVariant(
      const std::string& engine);

  cpp::result<std::vector<EngineVariantResponse>, std::string>
  GetInstalledEngineVariants(const std::string& engine) const;

  bool IsEngineLoaded(const std::string& engine) const;

  cpp::result<EngineV, std::string> GetLoadedEngine(
      const std::string& engine_name);

  std::vector<EngineV> GetLoadedEngines();

  cpp::result<void, std::string> LoadEngine(const std::string& engine_name);

  cpp::result<void, std::string> UnloadEngine(const std::string& engine_name);

  cpp::result<github_release_utils::GitHubRelease, std::string>
  GetLatestEngineVersion(const std::string& engine) const;

  cpp::result<bool, std::string> UnzipEngine(const std::string& engine,
                                             const std::string& version,
                                             const std::string& path);

  cpp::result<EngineUpdateResult, std::string> UpdateEngine(
      const std::string& engine);

 private:
  cpp::result<void, std::string> DownloadEngine(
      const std::string& engine, const std::string& version = "latest",
      const std::optional<std::string> variant_name = std::nullopt);

  cpp::result<bool, std::string> DownloadCuda(const std::string& engine,
                                              bool async = false);

  std::string GetMatchedVariant(const std::string& engine,
                                const std::vector<std::string>& variants);

  cpp::result<bool, std::string> IsEngineVariantReady(
      const std::string& engine, const std::string& version,
      const std::string& variant);

  std::shared_ptr<DownloadService> download_service_;

  struct HardwareInfo {
    std::unique_ptr<system_info_utils::SystemInfo> sys_inf;
    cortex::cpuid::CpuInfo cpu_inf;
    std::string cuda_driver_version;
  };
  HardwareInfo hw_inf_;
};
