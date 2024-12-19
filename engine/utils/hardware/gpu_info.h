#pragma once

#include "common/hardware_common.h"
#include "hwinfo/hwinfo.h"
#include "utils/hardware/gpu/vulkan/vulkan_gpu.h"
#include "utils/system_info_utils.h"

namespace cortex::hw {

inline std::vector<GPU> GetGPUInfo() {
  std::vector<GPU> res;
  // Only support for nvidia for now
  // auto gpus = hwinfo::getAllGPUs();
  auto nvidia_gpus = system_info_utils::GetGpuInfoList();
  auto vulkan_gpus = GetGpuInfoList().value_or(std::vector<cortex::hw::GPU>{});
  // add more information for GPUs

  for (size_t i = 0; i < nvidia_gpus.size(); i++) {
    for (size_t j = 0; j < vulkan_gpus.size(); j++) {
      if (nvidia_gpus[i].uuid == vulkan_gpus[j].uuid) {
        vulkan_gpus[j].version =
            nvidia_gpus[0].cuda_driver_version.value_or("unknown");
        vulkan_gpus[j].add_info = NvidiaAddInfo{
            .driver_version = nvidia_gpus[i].driver_version.value_or("unknown"),
            .compute_cap = nvidia_gpus[i].compute_cap.value_or("unknown")};
        vulkan_gpus[j].free_vram = std::stoll(nvidia_gpus[i].vram_free);
        vulkan_gpus[j].total_vram = std::stoll(nvidia_gpus[i].vram_total);
      }
    }
  }
  return vulkan_gpus;
}
}  // namespace cortex::hw