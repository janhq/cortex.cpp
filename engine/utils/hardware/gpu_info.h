#pragma once

#include "common/hardware_common.h"
#include "hwinfo/hwinfo.h"
#include "utils/system_info_utils.h"

namespace cortex::hw {

inline std::vector<GPU> GetGPUInfo() {
  std::vector<GPU> res;
  // Only support for nvidia for now
  // auto gpus = hwinfo::getAllGPUs();
  auto nvidia_gpus = system_info_utils::GetGpuInfoList();
  auto cuda_version = system_info_utils::GetCudaVersion();
  for (auto& n : nvidia_gpus) {
    res.emplace_back(
        GPU{.id = n.id,
            .name = n.name,
            .version = cuda_version,
            .add_info =
                NvidiaAddInfo{
                    .driver_version = n.driver_version.value_or("unknown"),
                    .compute_cap = n.compute_cap.value_or("unknown")},
            .free_vram = std::stoi(n.vram_free),
            .total_vram = std::stoi(n.vram_total),
            .uuid = n.uuid});
  }
  return res;
}
}  // namespace cortex::hw