#pragma once

#include <json/json.h>
#include <string>
#include <string_view>
#include <vector>
#include "common/hardware_common.h"
#include "cpu_usage.h"
#include "hwinfo/hwinfo.h"
#include "utils/cpuid/cpu_info.h"

namespace cortex::hw {
inline CPU GetCPUInfo() {
  auto res = hwinfo::getAllCPUs();
  if (res.empty())
    return CPU{};
  auto cpu = res[0];
  cortex::cpuid::CpuInfo inst;

#if defined(__linux__)
  float usage = 0;
  for (auto const& c : res) {
    usage += c.currentUtilisation();
  }
  usage = usage / res.size() * 100;
#else
  float usage = GetCPUUsage();
#endif
  // float usage = 0;
  return CPU{.cores = cpu.numPhysicalCores(),
             .arch = std::string(GetArch()),
             .model = cpu.modelName(),
             .usage = usage,
             .instructions = inst.instructions()};
}
}  // namespace cortex::hw