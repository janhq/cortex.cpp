#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/engine_servicei.h"
#include "cortex-common/EngineI.h"
#include "cortex-common/remote_enginei.h"
#include "database/engines.h"
#include "services/database_service.h"
#include "services/download_service.h"
#include "utils/cpuid/cpu_info.h"
#include "utils/dylib.h"
#include "utils/dylib_path_manager.h"
#include "utils/github_release_utils.h"
#include "utils/result.hpp"
#include "utils/system_info_utils.h"
#include "utils/task_queue.h"

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

using EngineV = std::variant<EngineI*, RemoteEngineI*>;

class EngineService : public EngineServiceI {
 private:
  using EngineRelease = github_release_utils::GitHubRelease;
  using EngineVariant = github_release_utils::GitHubAsset;

  struct EngineInfo {
    std::unique_ptr<cortex_cpp::dylib> dl;
    EngineV engine;
  };

  std::mutex engines_mutex_;
  std::unordered_map<std::string, EngineInfo> engines_{};
  std::shared_ptr<DownloadService> download_service_;
  std::shared_ptr<cortex::DylibPathManager> dylib_path_manager_;

  struct HardwareInfo {
    std::unique_ptr<system_info_utils::SystemInfo> sys_inf;
    cortex::cpuid::CpuInfo cpu_inf;
    std::string cuda_driver_version;
  };
  HardwareInfo hw_inf_;
  std::shared_ptr<DatabaseService> db_service_ = nullptr;
  std::shared_ptr<cortex::TaskQueue> q_ = nullptr;

 public:
  explicit EngineService(
      std::shared_ptr<DownloadService> download_service,
      std::shared_ptr<cortex::DylibPathManager> dylib_path_manager,
      std::shared_ptr<DatabaseService> db_service,
      std::shared_ptr<cortex::TaskQueue> q)
      : download_service_{download_service},
        dylib_path_manager_{dylib_path_manager},
        hw_inf_{.sys_inf = system_info_utils::GetSystemInfo(),
                .cuda_driver_version =
                    system_info_utils::GetDriverAndCudaVersion().second},
        db_service_(db_service),
        q_(q) {}

  std::vector<EngineInfo> GetEngineInfoList() const;

  /**
   * Check if an engines is ready (have at least one variant installed)
   */
  cpp::result<bool, std::string> IsEngineReady(const std::string& engine);

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
      const std::string& engine, const std::string& version,
      bool filter_compatible_only = false) const;

  cpp::result<DefaultEngineVariant, std::string> SetDefaultEngineVariant(
      const std::string& engine, const std::string& version,
      const std::string& variant) override;

  cpp::result<DefaultEngineVariant, std::string> GetDefaultEngineVariant(
      const std::string& engine) override;

  cpp::result<std::vector<EngineVariantResponse>, std::string>
  GetInstalledEngineVariants(const std::string& engine) const override;

  cpp::result<EngineV, std::string> GetLoadedEngine(
      const std::string& engine_name);

  std::vector<EngineV> GetLoadedEngines();

  cpp::result<void, std::string> LoadEngine(
      const std::string& engine_name) override;
  cpp::result<void, std::string> UnloadEngine(
      const std::string& engine_name) override;

  cpp::result<github_release_utils::GitHubRelease, std::string>
  GetLatestEngineVersion(const std::string& engine) const;

  cpp::result<bool, std::string> UnzipEngine(const std::string& engine,
                                             const std::string& version,
                                             const std::string& path);

  cpp::result<EngineUpdateResult, std::string> UpdateEngine(
      const std::string& engine);

  cpp::result<std::vector<cortex::db::EngineEntry>, std::string> GetEngines();

  cpp::result<cortex::db::EngineEntry, std::string> GetEngineById(int id);

  cpp::result<cortex::db::EngineEntry, std::string> GetEngineByNameAndVariant(
      const std::string& engine_name,
      const std::optional<std::string> variant = std::nullopt) const override;

  cpp::result<cortex::db::EngineEntry, std::string> UpsertEngine(
      const std::string& engine_name, const std::string& type,
      const std::string& api_key, const std::string& url,
      const std::string& version, const std::string& variant,
      const std::string& status, const std::string& metadata);

  std::string DeleteEngine(int id);

  cpp::result<Json::Value, std::string> GetRemoteModels(
      const std::string& engine_name);
  cpp::result<std::vector<std::string>, std::string> GetSupportedEngineNames();

  void RegisterEngineLibPath();

  bool IsRemoteEngine(const std::string& engine_name) const override;

  cpp::result<std::pair<std::filesystem::path, bool>, std::string>
  GetEngineDirPath(const std::string& engine_name);

 private:
  bool IsEngineLoaded(const std::string& engine);

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
};
