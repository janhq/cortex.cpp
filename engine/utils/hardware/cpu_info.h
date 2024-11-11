#pragma once

#include <json/json.h>
#include <string>
#include <string_view>
#include <vector>
#include "hwinfo/hwinfo.h"
#include "utils/cpuid/cpu_info.h"

namespace hardware {
namespace {
inline constexpr std::string_view GetArch() {
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
    defined(__amd64) || defined(__x86_64) || defined(_M_AMD64)
  return "amd64";
#elif defined(__arm__) || defined(__arm) || defined(__arm64__) || \
    defined(__aarch64__) || defined(__thumb__) ||                 \
    defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB) || \
    defined(_ARM) || defined(_M_ARM) || defined(_M_ARMT)
  return "arm64";
#else
  return "Unsupported";
#endif
}
}  // namespace
struct CPU {
  int cores;
  std::string arch;
  std::string model;
  std::vector<std::string> instructions;
};

inline Json::Value ToJson(const CPU& cpu) {
  Json::Value res;
  res["arch"] = cpu.arch;
  res["cores"] = cpu.cores;
  res["model"] = cpu.model;
  Json::Value insts(Json::arrayValue);
  for (auto const& i : cpu.instructions) {
    insts.append(i);
  }
  res["instructions"] = insts;
  return res;
}

namespace cpu {
inline CPU FromJson(const Json::Value& root) {
  int cores = root["cores"].asInt();
  std::string arch = root["arch"].asString();
  std::string model = root["model"].asString();
  std::vector<std::string> insts;
  for (auto const& i : root["instructions"]) {
    insts.emplace_back(i.asString());
  }
  return {.cores = cores, .arch = arch, .model = model, .instructions = insts};
}
}  // namespace cpu

inline CPU GetCPUInfo() {
  auto cpu = hwinfo::getAllCPUs()[0];
  cortex::cpuid::CpuInfo inst;
  return CPU{.cores = cpu.numPhysicalCores(),
             .arch = std::string(GetArch()),
             .model = cpu.modelName(),
             .instructions = inst.instructions()};
}
}  // namespace hardware