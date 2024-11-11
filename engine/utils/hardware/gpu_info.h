#pragma once
#include <json/json.h>
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
  std::string uuid;
  bool is_activated = true;
};

inline Json::Value ToJson(const std::vector<GPU>& gpus) {
  Json::Value res(Json::arrayValue);
  for (size_t i = 0; i < gpus.size(); i++) {
    Json::Value gpu;
    gpu["id"] = std::to_string(i);
    gpu["name"] = gpus[i].name;
    gpu["version"] = gpus[i].version;
    Json::Value add_info;
    if (std::holds_alternative<NvidiaAddInfo>(gpus[i].add_info)) {
      auto& v = std::get<NvidiaAddInfo>(gpus[i].add_info);
      add_info["driver_version"] = v.driver_version;
      add_info["compute_cap"] = v.compute_cap;
    }
    gpu["additional_information"] = add_info;

    gpu["free_vram"] = gpus[i].free_vram;
    gpu["total_vram"] = gpus[i].total_vram;
    gpu["uuid"] = gpus[i].uuid;
    gpu["activated"] = gpus[i].is_activated;
    res.append(gpu);
  }
  return res;
}

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
}  // namespace hardware