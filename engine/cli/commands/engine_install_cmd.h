#pragma once

#include <string>
#include "services/engine_service.h"

namespace commands {

class EngineInstallCmd {
 public:
  explicit EngineInstallCmd(std::shared_ptr<DownloadService> download_service,
                            const std::string& host, int port, bool show_menu)
      : engine_service_{EngineService(download_service)},
        host_(host),
        port_(port),
        show_menu_(show_menu),
        hw_inf_{.sys_inf = system_info_utils::GetSystemInfo(),
                .cuda_driver_version =
                    system_info_utils::GetDriverAndCudaVersion().second} {};

  bool Exec(const std::string& engine, const std::string& version = "latest",
            const std::string& src = "");

 private:
  EngineService engine_service_;
  std::string host_;
  int port_;
  bool show_menu_;

  struct HardwareInfo {
    std::unique_ptr<system_info_utils::SystemInfo> sys_inf;
    cortex::cpuid::CpuInfo cpu_inf;
    std::string cuda_driver_version;
  };
  HardwareInfo hw_inf_;
};
}  // namespace commands
