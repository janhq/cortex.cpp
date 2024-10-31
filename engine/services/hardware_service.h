#pragma once
#include <stdint.h>
#include <string>
#include <vector>

#include "utils/hardware/cpu_info.h"
#include "utils/hardware/gpu_info.h"
#include "utils/hardware/os_info.h"
#include "utils/hardware/power_info.h"
#include "utils/hardware/ram_info.h"
#include "utils/hardware/storage_info.h"

namespace services {

struct HardwareInfo {
  hardware::CPU cpu;
  hardware::OS os;
  hardware::Memory ram;
  hardware::StorageInfo storage;
  std::vector<hardware::GPU> gpus;
  hardware::PowerInfo power;
};

class HardwareService {
 public:
  HardwareInfo GetHardwareInfo();
};
}  // namespace services
