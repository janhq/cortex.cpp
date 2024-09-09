#pragma once

#include <trantor/utils/Logger.h>
#include <regex>
#include <sstream>
#include <vector>
#include "utils/command_executor.h"
#include "utils/logging_utils.h"
#ifdef _WIN32
#include <windows.h>
#endif

namespace system_info_utils {

constexpr static auto kUnsupported{"Unsupported"};
constexpr static auto kCudaVersionRegex{R"(CUDA Version:\s*([\d\.]+))"};
constexpr static auto kDriverVersionRegex{R"(Driver Version:\s*(\d+\.\d+))"};
constexpr static auto kGpuQueryCommand{
    "nvidia-smi --query-gpu=index,memory.total,name,compute_cap "
    "--format=csv,noheader,nounits"};
constexpr static auto kGpuInfoRegex{
    R"((\d+),\s*(\d+),\s*([^,]+),\s*([\d\.]+))"};

struct SystemInfo {
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

inline SystemInfo GetSystemInfo() {
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
  os << "mac";
#elif defined(__linux__)
  os << "linux";
#elif defined(_WIN32)
  os << "windows";
#else
  os << kUnsupported;
#endif
  return SystemInfo{os.str(), arch.str()};
}

inline bool IsNvidiaSmiAvailable() {
#ifdef _WIN32
  // Check if nvidia-smi.exe exists in the PATH on Windows
  char buffer[MAX_PATH];
  if (SearchPath(NULL, "nvidia-smi.exe", NULL, MAX_PATH, buffer, NULL) != 0) {
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

inline std::string GetDriverVersion() {
  if (!IsNvidiaSmiAvailable()) {
    CTL_INF("nvidia-smi is not available!");
    return "";
  }
  try {
    CommandExecutor cmd("nvidia-smi");
    auto output = cmd.execute();

    const std::regex driver_version_reg(kDriverVersionRegex);
    std::smatch match;

    if (std::regex_search(output, match, driver_version_reg)) {
      LOG_INFO << "Gpu Driver Version: " << match[1].str();
      return match[1].str();
    } else {
      LOG_ERROR << "Gpu Driver not found!";
      return "";
    }
  } catch (const std::exception& e) {
    LOG_ERROR << "Error: " << e.what();
    return "";
  }
}

inline std::string GetCudaVersion() {
  if (!IsNvidiaSmiAvailable()) {
    CTL_INF("nvidia-smi is not available!");
    return "";
  }
  try {
    CommandExecutor cmd("nvidia-smi");
    auto output = cmd.execute();

    const std::regex cuda_version_reg(kCudaVersionRegex);
    std::smatch match;

    if (std::regex_search(output, match, cuda_version_reg)) {
      LOG_INFO << "CUDA Version: " << match[1].str();
      return match[1].str();
    } else {
      LOG_ERROR << "CUDA Version not found!";
      return "";
    }
  } catch (const std::exception& e) {
    LOG_ERROR << "Error: " << e.what();
    return "";
  }
}

struct GpuInfo {
  std::string id;
  std::string vram;
  std::string name;
  std::string arch;
  // nvidia driver version. Haven't checked for AMD GPU.
  std::optional<std::string> driver_version;
  std::optional<std::string> cuda_driver_version;
  std::optional<std::string> compute_cap;
};

inline std::vector<GpuInfo> GetGpuInfoListVulkan() {
  std::vector<GpuInfo> gpuInfoList;

  try {
    // NOTE: current ly we don't have logic to download vulkaninfoSDK
#ifdef _WIN32
    CommandExecutor cmd("vulkaninfoSDK.exe --summary");
#else
    CommandExecutor cmd("vulkaninfoSDK --summary");
#endif
    auto output = cmd.execute();

    // Regular expression patterns to match each field
    std::regex gpu_block_reg(R"(GPU(\d+):)");
    std::regex field_pattern(R"(\s*(\w+)\s*=\s*(.*))");

    std::sregex_iterator iter(output.begin(), output.end(), gpu_block_reg);
    std::sregex_iterator end;

    while (iter != end) {
      GpuInfo gpuInfo;

      // Extract GPU ID from the GPU block pattern (e.g., GPU0 -> id = "0")
      gpuInfo.id = (*iter)[1].str();

      auto gpu_start_pos = iter->position(0) + iter->length(0);
      auto gpu_end_pos = std::next(iter) != end ? std::next(iter)->position(0)
                                                : std::string::npos;
      std::string gpu_block =
          output.substr(gpu_start_pos, gpu_end_pos - gpu_start_pos);

      std::sregex_iterator field_iter(gpu_block.begin(), gpu_block.end(),
                                      field_pattern);

      while (field_iter != end) {
        std::string key = (*field_iter)[1].str();
        std::string value = (*field_iter)[2].str();

        if (key == "deviceName")
          gpuInfo.name = value;
        else if (key == "apiVersion")
          gpuInfo.compute_cap = value;

        gpuInfo.vram = "";  // not available
        gpuInfo.arch = GetGpuArch(gpuInfo.name);

        ++field_iter;
      }

      gpuInfoList.push_back(gpuInfo);
      ++iter;
    }
  } catch (const std::exception& e) {
    LOG_ERROR << "Error: " << e.what();
  }

  return gpuInfoList;
}

inline std::vector<GpuInfo> GetGpuInfoList() {
  std::vector<GpuInfo> gpuInfoList;

  try {
    // TODO: improve by parsing both in one command execution
    auto driver_version = GetDriverVersion();
    auto cuda_version = GetCudaVersion();

    CommandExecutor cmd(kGpuQueryCommand);
    auto output = cmd.execute();

    const std::regex gpu_info_reg(kGpuInfoRegex);
    std::smatch match;
    std::string::const_iterator search_start(output.cbegin());

    while (
        std::regex_search(search_start, output.cend(), match, gpu_info_reg)) {
      GpuInfo gpuInfo = {
          match[1].str(),              // id
          match[2].str(),              // vram
          match[3].str(),              // name
          GetGpuArch(match[3].str()),  // arch
          driver_version,              // driver_version
          cuda_version,                // cuda_driver_version
          match[4].str()               // compute_cap
      };
      gpuInfoList.push_back(gpuInfo);
      search_start = match.suffix().first;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return gpuInfoList;
}
}  // namespace system_info_utils
