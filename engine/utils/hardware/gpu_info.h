#pragma once
#include <string>
#include <variant>
#include <vector>
#include "hwinfo/hwinfo.h"
#include "utils/system_info_utils.h"

namespace hardware {
// This can be different depends on gpu types
struct NvidiaAddInfo {
  std::string driver_version;
  std::string compute_cap;
};
struct AmdAddInfo {};
using GPUAddInfo = std::variant<NvidiaAddInfo, AmdAddInfo>;
struct GPU {
  std::string id;
  std::string name;
  std::string version;
  GPUAddInfo add_info;
  int64_t free_vram;
  int64_t total_vram;
};

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
            .total_vram = std::stoi(n.vram_total)});
  }
  return res;
}
}  // namespace hardware