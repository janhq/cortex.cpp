#pragma once

#include <trantor/utils/Logger.h>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <vector>
#include "utils/command_executor.h"
#include "utils/engine_constants.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace system_info_utils {

constexpr static auto kUnsupported{"Unsupported"};
constexpr static auto kCudaVersionRegex{R"(CUDA Version:\s*([\d\.]+))"};
constexpr static auto kDriverVersionRegex{R"(Driver Version:\s*(\d+\.\d+))"};
constexpr static auto kGpuQueryCommand{
    "nvidia-smi "
    "--query-gpu=index,memory.total,memory.free,name,compute_cap,uuid "
    "--format=csv,noheader,nounits"};
constexpr static auto kGpuInfoRegex{
    R"((\d+),\s*(\d+),\s*(\d+),\s*([^,]+),\s*([\d\.]+),\s*([^\n,]+))"};

constexpr static auto kGpuQueryCommandFb{
    "nvidia-smi "
    "--query-gpu=index,memory.total,memory.free,name,uuid "
    "--format=csv,noheader,nounits"};
constexpr static auto kGpuInfoRegexFb{
    R"((\d+),\s*(\d+),\s*(\d+),\s*([^,]+),\s*([^\n,]+))"};

struct SystemInfo {
  explicit SystemInfo(std::string os, std::string arch)
      : os(std::move(os)), arch(std::move(arch)) {}
  std::string os;
  std::string arch;
};

/**
 * @brief Get the Gpu Arch. Currently we only support Ampere and Ada.
 * Might need to come up with better way to detect the GPU architecture.
 * 
 * @param gpuName E.g. NVIDIA GeForce RTX 4090
 * @return corresponding GPU arch. E.g. ampere, ada.
 */
inline std::string GetGpuArch(const std::string& gpuName) {
  std::string lowerGpuName = gpuName;
  std::transform(lowerGpuName.begin(), lowerGpuName.end(), lowerGpuName.begin(),
                 ::tolower);

  if (lowerGpuName.find("nvidia") == std::string::npos) {
    return "unknown";
  }

  if (gpuName.find("30") != std::string::npos) {
    return "ampere";
  } else if (gpuName.find("40") != std::string::npos) {
    return "ada";
  } else {
    return "unknown";
  }
}

inline std::unique_ptr<SystemInfo> GetSystemInfo() {
  std::ostringstream arch;
  std::ostringstream os;

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
    defined(__amd64) || defined(__x86_64) || defined(_M_AMD64)
  arch << "amd64";
#elif defined(__arm__) || defined(__arm) || defined(__arm64__) || \
    defined(__aarch64__) || defined(__thumb__) ||                 \
    defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB) || \
    defined(_ARM) || defined(_M_ARM) || defined(_M_ARMT)
  arch << "arm64";
#else
  arch << kUnsupported;
#endif

#if defined(__APPLE__) && defined(__MACH__)
  os << kMacOs;
#elif defined(__linux__)
  os << kLinuxOs;
#elif defined(_WIN32)
  os << kWindowsOs;
#else
  os << kUnsupportedOs;
#endif
  return std::make_unique<SystemInfo>(os.str(), arch.str());
}

inline bool IsNvidiaSmiAvailable() {
#ifdef _WIN32
  // Check if nvidia-smi.exe exists in the PATH on Windows
  wchar_t buffer[MAX_PATH];
  if (SearchPath(NULL, L"nvidia-smi.exe", NULL, MAX_PATH, buffer, NULL) != 0) {
    return true;
  } else {
    return false;
  }
#else
  // Check if nvidia-smi is available on Unix-like systems
  int result = std::system("which nvidia-smi > /dev/null 2>&1");
  return result == 0;
#endif
}

std::pair<std::string, std::string> GetDriverAndCudaVersion();

struct GpuInfo {
  std::string id;
  std::string vram_total;
  std::string vram_free;
  std::string name;
  std::string arch;
  // nvidia driver version. Haven't checked for AMD GPU.
  std::optional<std::string> driver_version;
  std::optional<std::string> cuda_driver_version;
  std::optional<std::string> compute_cap;
  std::string uuid;
  std::string vendor;
};

std::vector<GpuInfo> GetGpuInfoListVulkan();

std::vector<GpuInfo> GetGpuInfoList();
}  // namespace system_info_utils
