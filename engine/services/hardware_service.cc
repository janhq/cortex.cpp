#include "hardware_service.h"

namespace services {
HardwareInfo HardwareService::GetHardwareInfo() {
  return HardwareInfo{.cpu = hardware::GetCPUInfo(),
                      .os = hardware::GetOSInfo(),
                      .ram = hardware::GetMemoryInfo(),
                      .storage = hardware::GetStorageInfo(),
                      .gpus = hardware::GetGPUInfo(),
                      .power = hardware::GetPowerInfo()};
}
}  // namespace services