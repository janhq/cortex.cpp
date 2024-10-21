#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "services/download_service.h"
#include "utils/cpuid/cpu_info.h"
#include "utils/engine_constants.h"
#include "utils/result.hpp"
#include "utils/system_info_utils.h"

struct EngineInfo {
  std::string name;
  std::string description;
  std::string format;
  std::optional<std::string> version;
  std::string product_name;
  std::string status;
  std::optional<std::string> variant;
};

namespace system_info_utils {
struct SystemInfo;
}
class EngineService {
 public:
  constexpr static auto kIncompatible = "Incompatible";
  constexpr static auto kReady = "Ready";
  constexpr static auto kNotInstalled = "Not Installed";

  const std::vector<std::string_view> kSupportEngines = {
      kLlamaEngine, kOnnxEngine, kTrtLlmEngine};

  explicit EngineService(std::shared_ptr<DownloadService> download_service)
      : download_service_{download_service},
        hw_inf_{.sys_inf = system_info_utils::GetSystemInfo(),
                .cuda_driver_version = system_info_utils::GetCudaVersion()} {}

  cpp::result<EngineInfo, std::string> GetEngineInfo(
      const std::string& engine) const;

  std::vector<EngineInfo> GetEngineInfoList() const;

  cpp::result<bool, std::string> InstallEngine(
      const std::string& engine, const std::string& version = "latest",
      const std::string& src = "");

  cpp::result<bool, std::string> InstallEngineAsync(
      const std::string& engine, const std::string& version = "latest",
      const std::string& src = "");

  cpp::result<bool, std::string> UninstallEngine(const std::string& engine);

 private:
  cpp::result<bool, std::string> UnzipEngine(const std::string& engine,
                                             const std::string& version,
                                             const std::string& path);

  cpp::result<bool, std::string> DownloadEngine(
      const std::string& engine, const std::string& version = "latest", bool async = false);

  cpp::result<bool, std::string> DownloadCuda(const std::string& engine, bool async = false);

  std::string GetMatchedVariant(const std::string& engine,
                                const std::vector<std::string>& variants);

 private:
  std::shared_ptr<DownloadService> download_service_;

  struct HardwareInfo {
    std::unique_ptr<system_info_utils::SystemInfo> sys_inf;
    cortex::cpuid::CpuInfo cpu_inf;
    std::string cuda_driver_version;
  };
  HardwareInfo hw_inf_;
};
