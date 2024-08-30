#pragma once

#include <trantor/utils/Logger.h>
#include <regex>
#include <vector>
#include "sstream"
#include "utils/command_executor.h"
#include "optional"
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

const std::unordered_map<std::string, std::string> kGpuArchMap{
    {"10.0", "blackwell"}, {"9.0", "hopper"}, {"8.9", "ada"},
    {"8.6", "ampere"},     {"8.7", "ampere"}, {"8.0", "ampere"},
    {"7.5", "turing"},     {"7.2", "volta"},  {"7.0", "volta"},
    {"6.2", "pascal"},     {"6.1", "pascal"}, {"6.0", "pascal"}};

/**
 * @brief Get the Gpu Arch. Currently we only support Ampere and Ada.
 * Might need to come up with better way to detect the GPU architecture.
 * 
 * @param gpuName E.g. NVIDIA GeForce RTX 4090
 * @return corresponding GPU arch. E.g. ampere, ada.
 */
inline std::string GetGpuArch(const std::string& gpu_name,
                              const std::string& compute_cap) {
  std::string lowerGpuName = gpu_name;
  std::transform(lowerGpuName.begin(), lowerGpuName.end(), lowerGpuName.begin(),
                 ::tolower);

  if (lowerGpuName.find("nvidia") == std::string::npos) {
    return "unknown";
  }

  auto it = kGpuArchMap.find(compute_cap);
  if (it != kGpuArchMap.end()) {
    return it->second;
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

constexpr auto vulkan_sample_output = R"(
==========
VULKANINFO
==========

Vulkan Instance Version: 1.3.280


Instance Extensions: count = 19
-------------------------------
VK_EXT_debug_report                    : extension revision 10
VK_EXT_debug_utils                     : extension revision 2
VK_EXT_direct_mode_display             : extension revision 1
VK_EXT_surface_maintenance1            : extension revision 1
VK_EXT_swapchain_colorspace            : extension revision 4
VK_KHR_device_group_creation           : extension revision 1
VK_KHR_display                         : extension revision 23
VK_KHR_external_fence_capabilities     : extension revision 1
VK_KHR_external_memory_capabilities    : extension revision 1
VK_KHR_external_semaphore_capabilities : extension revision 1
VK_KHR_get_display_properties2         : extension revision 1
VK_KHR_get_physical_device_properties2 : extension revision 2
VK_KHR_get_surface_capabilities2       : extension revision 1
VK_KHR_portability_enumeration         : extension revision 1
VK_KHR_surface                         : extension revision 25
VK_KHR_surface_protected_capabilities  : extension revision 1
VK_KHR_win32_surface                   : extension revision 6
VK_LUNARG_direct_driver_loading        : extension revision 1
VK_NV_external_memory_capabilities     : extension revision 1

Instance Layers: count = 1
--------------------------
VK_LAYER_NV_optimus NVIDIA Optimus layer 1.3.280  version 1

Devices:
========
GPU0:
        apiVersion         = 1.3.280
        driverVersion      = 560.70.0.0
        vendorID           = 0x10de
        deviceID           = 0x2684
        deviceType         = PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        deviceName         = NVIDIA GeForce RTX 4090
        driverID           = DRIVER_ID_NVIDIA_PROPRIETARY
        driverName         = NVIDIA
        driverInfo         = 560.70
        conformanceVersion = 1.3.8.2
        deviceUUID         = 11deafdf-9f15-e857-2a87-8acc153fc9f7
        driverUUID         = 10f251d9-d3c0-5001-bf67-24bb06423040
)";

constexpr auto gpu_query_list_sample_output = R"(
0, 46068, NVIDIA RTX A6000, 8.6
1, 46068, NVIDIA RTX A6000, 8.6
)";

constexpr auto nvidia_smi_sample_output = R"(
Sun Aug 25 22:29:25 2024
+-----------------------------------------------------------------------------------------+
| NVIDIA-SMI 560.70                 Driver Version: 560.70         CUDA Version: 12.6     |
|-----------------------------------------+------------------------+----------------------+
| GPU  Name                  Driver-Model | Bus-Id          Disp.A | Volatile Uncorr. ECC |
| Fan  Temp   Perf          Pwr:Usage/Cap |           Memory-Usage | GPU-Util  Compute M. |
|                                         |                        |               MIG M. |
|=========================================+========================+======================|
|   0  NVIDIA GeForce RTX 4090      WDDM  |   00000000:01:00.0 Off |                  Off |
|  0%   24C    P8             10W /  500W |     395MiB /  24564MiB |     19%      Default |
|                                         |                        |                  N/A |
+-----------------------------------------+------------------------+----------------------+

+-----------------------------------------------------------------------------------------+
| Processes:                                                                              |
|  GPU   GI   CI        PID   Type   Process name                              GPU Memory |
|        ID   ID                                                               Usage      |
|=========================================================================================|
|    0   N/A  N/A      3984    C+G   ...5n1h2txyewy\ShellExperienceHost.exe      N/A      |
|    0   N/A  N/A      7904    C+G   ...ekyb3d8bbwe\PhoneExperienceHost.exe      N/A      |
|    0   N/A  N/A      8240    C+G   ...__8wekyb3d8bbwe\WindowsTerminal.exe      N/A      |
|    0   N/A  N/A      8904    C+G   C:\Windows\explorer.exe                     N/A      |
|    0   N/A  N/A      9304    C+G   ...siveControlPanel\SystemSettings.exe      N/A      |
|    0   N/A  N/A      9944    C+G   ...nt.CBS_cw5n1h2txyewy\SearchHost.exe      N/A      |
|    0   N/A  N/A     11140    C+G   ...2txyewy\StartMenuExperienceHost.exe      N/A      |
+-----------------------------------------------------------------------------------------+
)";

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
    LOG_INFO << "nvidia-smi is not available!";
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
    LOG_INFO << "nvidia-smi is not available!";
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
  std::optional<std::string> arch;
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

        if (key == "deviceName") {
          gpuInfo.name = value;
        }
        gpuInfo.vram = "";  // not available

        ++field_iter;
      }

      gpuInfoList.push_back(gpuInfo);
      ++iter;
    }
  } catch (const std::exception& e) {}

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
          match[1].str(),                              // id
          match[2].str(),                              // vram
          match[3].str(),                              // name
          GetGpuArch(match[3].str(), match[4].str()),  // arch
          driver_version,                              // driver_version
          cuda_version,                                // cuda_driver_version
          match[4].str()                               // compute_cap
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
