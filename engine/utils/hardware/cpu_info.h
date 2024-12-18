#pragma once

#include <json/json.h>
#include <string>
#include <string_view>
#include <vector>
#include "common/hardware_common.h"
#include "hwinfo/hwinfo.h"
#include "utils/cpuid/cpu_info.h"

namespace cortex::hw {
inline CPU GetCPUInfo() {
  auto res = hwinfo::getAllCPUs();
  if (res.empty())
    return CPU{};
  auto cpu = res[0];
  cortex::cpuid::CpuInfo inst;
  return CPU{.cores = cpu.numPhysicalCores(),
             .arch = std::string(GetArch()),
             .model = cpu.modelName(),
             .instructions = inst.instructions()};
}
}  // namespace cortex::hw