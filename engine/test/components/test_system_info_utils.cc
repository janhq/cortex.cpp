#include <string>
#include "gtest/gtest.h"

class SystemInfoUtilsTestSuite : public ::testing::Test {
 protected:
  const std::string nvidia_smi_sample_output = R"(
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

  const std::string vulkan_sample_output = R"(
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

  const std::string gpu_query_list_sample_output = R"(
0, 46068, NVIDIA RTX A6000, 8.6
1, 46068, NVIDIA RTX A6000, 8.6
)";
};
