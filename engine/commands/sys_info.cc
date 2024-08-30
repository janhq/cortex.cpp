#include "sys_info.h"
#include "trantor/utils/Logger.h"
#include "utils/system_info_utils.h"

namespace commands {

void SysInfoCmd::Exec() {
  LOG_INFO << "SysInfoCmd::Exec()";
  auto gpuInfoList = system_info_utils::GetGpuInfoList();
  // print gpu info
  for (auto& gpuInfo : gpuInfoList) {
    LOG_INFO << "GPU: " << gpuInfo.name << " (" << gpuInfo.id << ")";
    if (gpuInfo.arch.has_value()) {
      LOG_INFO << "  Arch: " << gpuInfo.arch.value();
    }
    LOG_INFO << "  Memory: " << gpuInfo.vram;
    if (gpuInfo.compute_cap.has_value()) {
      LOG_INFO << "  Compute capability: " << gpuInfo.compute_cap.value();
    }
  }
}
};  // namespace commands
