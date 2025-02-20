#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <mutex>

#include "common/hardware_config.h"
#include "database_service.h"
#include "utils/hardware/cpu_info.h"
#include "utils/hardware/gpu_info.h"
#include "utils/hardware/os_info.h"
#include "utils/hardware/power_info.h"
#include "utils/hardware/ram_info.h"
#include "utils/hardware/storage_info.h"

struct HardwareInfo {
  cortex::hw::CPU cpu;
  cortex::hw::OS os;
  cortex::hw::Memory ram;
  cortex::hw::StorageInfo storage;
  std::vector<cortex::hw::GPU> gpus;
  cortex::hw::PowerInfo power;
};

class HardwareService {
 public:
  explicit HardwareService(std::shared_ptr<DatabaseService> db_service)
      : db_service_(db_service) {}
  HardwareInfo GetHardwareInfo();
  bool Restart(const std::string& host, int port);
  bool SetActivateHardwareConfig(const cortex::hw::ActivateHardwareConfig& ahc);
  bool ShouldRestart() const { return !!ahc_; }
  void UpdateHardwareInfos();
  bool IsValidConfig(const cortex::hw::ActivateHardwareConfig& ahc);

 private:
  void CheckDependencies();
  std::vector<int> GetCudaConfig();

 private:
  std::shared_ptr<DatabaseService> db_service_ = nullptr;
  std::optional<cortex::hw::ActivateHardwareConfig> ahc_;
  std::mutex mtx_;
};