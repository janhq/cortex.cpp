#pragma once

#include "common/hardware_common.h"
#include "hwinfo/hwinfo.h"
#include "utils/hardware/gpu/vulkan/vulkan_gpu.h"
#include "utils/system_info_utils.h"

namespace cortex::hw {

inline std::vector<GPU> GetGPUInfo() {
  auto nvidia_gpus = system_info_utils::GetGpuInfoList();
  auto vulkan_gpus = VulkanGpu::GetInstance().GetGpuInfoList().value_or(
      std::vector<cortex::hw::GPU>{});
  auto use_vulkan_info = nvidia_gpus.empty();

  // In case we have vulkan info, add more information for GPUs
  for (size_t i = 0; i < nvidia_gpus.size(); i++) {
    for (size_t j = 0; j < vulkan_gpus.size(); j++) {
      if (nvidia_gpus[i].uuid.find(vulkan_gpus[j].uuid) != std::string::npos) {
        use_vulkan_info = true;
        vulkan_gpus[j].version =
            nvidia_gpus[0].cuda_driver_version.value_or("unknown");
        vulkan_gpus[j].add_info = NvidiaAddInfo{
            nvidia_gpus[i].driver_version.value_or("unknown"),  //driver_version
            nvidia_gpus[i].compute_cap.value_or("unknown")};    //compute_cap
        vulkan_gpus[j].free_vram = std::stoll(nvidia_gpus[i].vram_free);
        vulkan_gpus[j].total_vram = std::stoll(nvidia_gpus[i].vram_total);
        vulkan_gpus[j].vendor = nvidia_gpus[i].vendor;
      }
    }
  }

  // Erase invalid GPUs
  for (std::vector<cortex::hw::GPU>::iterator it = vulkan_gpus.begin();
       it != vulkan_gpus.end();) {
    if ((*it).total_vram <= 0)
      it = vulkan_gpus.erase(it);
    else
      ++it;
  }

  if (use_vulkan_info) {
    return vulkan_gpus;
  } else {
    std::vector<GPU> res;
    for (auto& n : nvidia_gpus) {
      res.emplace_back(GPU{
          n.id,    // id
          0,       // device_id (not specified in original)
          n.name,  // name
          nvidia_gpus[0].cuda_driver_version.value_or("unknown"),  // version
          NvidiaAddInfo{
              // add_info
              n.driver_version.value_or("unknown"),  // driver_version
              n.compute_cap.value_or("unknown")      // compute_cap
          },
          std::stoi(n.vram_free),    // free_vram
          std::stoi(n.vram_total),   // total_vram
          n.uuid,                    // uuid
          true,                      // is_activated (default value)
          n.vendor,                  // vendor
          GpuType::kGpuTypeDiscrete  // gpu_type
      });
    }
    return res;
  }
}
}  // namespace cortex::hw
