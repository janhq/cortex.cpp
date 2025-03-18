#pragma once
#include <assert.h>
#include <json/json.h>
#include <string>
#include <variant>
#include <vector>

namespace cortex::hw {

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
  float usage;
  std::vector<std::string> instructions;
};

inline Json::Value ToJson(const CPU& cpu) {
  Json::Value res;
  res["arch"] = cpu.arch;
  res["cores"] = cpu.cores;
  res["model"] = cpu.model;
  res["usage"] = cpu.usage;
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
  float usage = root["usage"].asFloat();
  std::vector<std::string> insts;
  for (auto const& i : root["instructions"]) {
    insts.emplace_back(i.asString());
  }
  return {cores, arch, model, usage, insts};
}
}  // namespace cpu

// This can be different depends on gpu types
struct NvidiaAddInfo {
  std::string driver_version;
  std::string compute_cap;
};
struct AmdAddInfo {};
using GPUAddInfo = std::variant<NvidiaAddInfo, AmdAddInfo>;

enum class GpuType {
  kGpuTypeOther = 0,
  kGpuTypeIntegrated = 1,
  kGpuTypeDiscrete = 2,
  kGpuTypeVirtual = 3,
  kGpuTypeCpu = 4,
  kGpuTypeMaxEnum = 0x7FFFFFFF
};

struct GPU {
  std::string id;
  uint32_t device_id;
  std::string name;
  std::string version;
  GPUAddInfo add_info;
  int64_t free_vram;
  int64_t total_vram;
  std::string uuid;
  bool is_activated = true;
  std::string vendor;
  GpuType gpu_type;
};

inline Json::Value ToJson(const std::vector<GPU>& gpus) {
  Json::Value res(Json::arrayValue);
  for (size_t i = 0; i < gpus.size(); i++) {
    Json::Value gpu;
    gpu["id"] = gpus[i].id;
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
    gpu["vendor"] = gpus[i].vendor;
    if (gpus[i].total_vram > 0) {
      res.append(gpu);
    }
  }
  return res;
}

namespace gpu {
inline std::vector<GPU> FromJson(const Json::Value& root) {
  assert(root.isArray());
  std::vector<GPU> res;
  for (auto const& gpu_json : root) {
    GPU gpu;
    gpu.id = gpu_json["id"].asString();
    gpu.name = gpu_json["name"].asString();
    gpu.version = gpu_json["version"].asString();
    NvidiaAddInfo add_inf;
    add_inf.driver_version =
        gpu_json["additional_information"]["driver_version"].asString();
    add_inf.compute_cap =
        gpu_json["additional_information"]["compute_cap"].asString();
    gpu.add_info = add_inf;
    gpu.free_vram = gpu_json["free_vram"].asInt64();
    gpu.total_vram = gpu_json["total_vram"].asInt64();
    gpu.uuid = gpu_json["uuid"].asString();
    gpu.is_activated = gpu_json["activated"].asBool();
    res.emplace_back(gpu);
  }
  return res;
}
}  // namespace gpu

struct OS {
  std::string name;
  std::string version;
  std::string arch;
};

inline Json::Value ToJson(const OS& os) {
  Json::Value res;
  res["version"] = os.version;
  res["name"] = os.name;
  return res;
}

namespace os {
inline OS FromJson(const Json::Value& root) {
  return {root["name"].asString(), root["version"].asString(), ""};
}
}  // namespace os

struct PowerInfo {
  std::string charging_status;
  int battery_life;
  bool is_power_saving;
};

inline Json::Value ToJson(const PowerInfo& pi) {
  Json::Value res;
  res["charging_status"] = pi.charging_status;
  res["battery_life"] = pi.battery_life;
  res["is_power_saving"] = pi.is_power_saving;
  return res;
}

namespace power {
inline PowerInfo FromJson(const Json::Value& root) {
  return {root["charging_status"].asString(), root["battery_life"].asInt(),
          root["is_power_saving"].asBool()};
}
}  // namespace power

namespace {
int64_t ByteToMiB(int64_t b) {
  return b / 1024 / 1024;
}
}  // namespace
struct Memory {
  int64_t total_MiB;
  int64_t available_MiB;
  std::string type;
};

inline Json::Value ToJson(const Memory& m) {
  Json::Value res;
  res["total"] = m.total_MiB;
  res["available"] = m.available_MiB;
  res["type"] = m.type;
  return res;
}

namespace memory {
inline Memory FromJson(const Json::Value& root) {
  return {root["total"].asInt64(), root["available"].asInt64(),
          root["type"].asString()};
}
}  // namespace memory

struct StorageInfo {
  std::string type;
  int64_t total;
  int64_t available;
};

inline Json::Value ToJson(const StorageInfo& si) {
  Json::Value res;
  res["total"] = si.total;
  res["available"] = si.available;
  res["type"] = si.type;
  return res;
}

namespace storage {
inline StorageInfo FromJson(const Json::Value& root) {
  return {root["type"].asString(), root["total"].asInt64(),
          root["available"].asInt64()};
}
}  // namespace storage
}  // namespace cortex::hw
