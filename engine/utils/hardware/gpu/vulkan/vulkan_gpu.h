#pragma once

#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/hardware_common.h"
#include "utils/result.hpp"
#include "vulkan.h"

#if defined(_WIN32)
#include "../adl/adl_sdk.h"
#include "../adl/amd_ags.h"
#else
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#endif

namespace cortex::hw {
#if defined(_WIN32)
// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int (*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int (*ADL_MAIN_CONTROL_DESTROY)();
typedef int (*ADL_ADAPTER_NUMBEROFADAPTERS_GET)(int*);
typedef int (*ADL_ADAPTER_ADAPTERINFO_GET)(LPAdapterInfo, int);
typedef int (*ADL_ADAPTER_ACTIVE_GET)(int, int*);
typedef int (*ADL_OVERDRIVE_CAPS)(int iAdapterIndex, int* iSupported,
                                  int* iEnabled, int* iVersion);
typedef int (*ADL_GET_DEDICATED_VRAM_USAGE)(ADL_CONTEXT_HANDLE context,
                                            int iAdapterIndex,
                                            int* vram_usage_in_MB);

// Memory allocation function
inline void* __stdcall ADL_Main_Memory_Alloc(int iSize) {
  void* lpBuffer = malloc(iSize);
  return lpBuffer;
}

// Optional Memory de-allocation function
inline void __stdcall ADL_Main_Memory_Free(void** lpBuffer) {
  if (nullptr != *lpBuffer) {
    free(*lpBuffer);
    *lpBuffer = nullptr;
  }
}

inline cpp::result<std::unordered_map<std::string, int>, std::string>
GetGpuUsage() {
  HINSTANCE hDLL;  // Handle to DLL

  LPAdapterInfo lpAdapterInfo = nullptr;
  int i;
  int num_adapters = 0;

  hDLL = LoadLibrary(L"atiadlxx.dll");
  if (hDLL == nullptr)
    // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
    // Try to load the 32 bit library (atiadlxy.dll) instead
    hDLL = LoadLibrary(L"atiadlxy.dll");

  if (nullptr == hDLL) {
    return cpp::fail("ADL library not found!");
  }

  auto ADL_Main_Control_Create =
      (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
  auto ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(
      hDLL, "ADL_Main_Control_Destroy");
  auto ADL_Adapter_NumberOfAdapters_Get =
      (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(
          hDLL, "ADL_Adapter_NumberOfAdapters_Get");
  auto ADL_Adapter_AdapterInfo_Get =
      (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(
          hDLL, "ADL_Adapter_AdapterInfo_Get");
  auto ADL_Adapter_Active_Get =
      (ADL_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL_Adapter_Active_Get");

  auto ADL_Get_Dedicated_Vram_Usage =
      (ADL_GET_DEDICATED_VRAM_USAGE)GetProcAddress(
          hDLL, "ADL2_Adapter_DedicatedVRAMUsage_Get");

  if (nullptr == ADL_Main_Control_Create ||
      nullptr == ADL_Main_Control_Destroy ||
      nullptr == ADL_Adapter_NumberOfAdapters_Get ||
      nullptr == ADL_Adapter_AdapterInfo_Get ||
      nullptr == ADL_Adapter_Active_Get ||
      nullptr == ADL_Get_Dedicated_Vram_Usage) {
    return cpp::fail("ADL's API is missing!");
  }

  // Initialize ADL. The second parameter is 1, which means:
  // retrieve adapter information only for adapters that are physically present and enabled in the system
  if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1)) {
    return cpp::fail("ADL Initialization Error!");
  }

  // Obtain the number of adapters for the system
  if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&num_adapters)) {
    return cpp::fail("Cannot get the number of adapters!");
  }
  // std::cout << "num_adapters: " << num_adapters << std::endl;

  if (0 < num_adapters) {
    lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo) * num_adapters);
    memset(lpAdapterInfo, '\0', sizeof(AdapterInfo) * num_adapters);

    // Get the AdapterInfo structure for all adapters in the system
    ADL_Adapter_AdapterInfo_Get(lpAdapterInfo,
                                sizeof(AdapterInfo) * num_adapters);
  }

  // Looking for first present and active adapter in the system
  int adapter_id = -1;
  std::unordered_map<std::string, int> vram_usages;
  for (i = 0; i < num_adapters; i++) {
    int adapter_active = 0;
    AdapterInfo adapter_info = lpAdapterInfo[i];
    ADL_Adapter_Active_Get(adapter_info.iAdapterIndex, &adapter_active);

    if (ADL_Get_Dedicated_Vram_Usage) {
      int vram_usage_in_MB = 0;
      ADL_Get_Dedicated_Vram_Usage(nullptr, i, &vram_usage_in_MB);
      vram_usages[adapter_info.strAdapterName] = vram_usage_in_MB;
    }
  }

  ADL_Main_Control_Destroy();
  return vram_usages;
}

#else

struct AmdGpuUsage {
  int64_t total_vram_MiB;
  int64_t used_vram_MiB;
};

inline cpp::result<std::unordered_map<int, AmdGpuUsage>, std::string>
GetGpuUsage() {
  // list all devices
  std::unordered_map<int, AmdGpuUsage> res;
  std::string path = "/sys/class/drm/";
  try {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
      auto const& es = entry.path().stem().string();
      if (entry.is_directory() && es.find("card") != std::string::npos &&
          es.find("-") == std::string::npos) {
        // std::cout << entry.path() << std::endl;
        std::filesystem::path gpu_device_path = entry.path() / "device";
        std::filesystem::path vendor_path = gpu_device_path / "vendor";
        std::ifstream vendor_file(vendor_path.string());
        if (vendor_file.is_open()) {
          std::string vendor_str;
          std::getline(vendor_file, vendor_str);
          vendor_file.close();

          // std::cout << "Vendor: " << vendor_str << std::endl;
          if (vendor_str == "0x1002") {
            std::filesystem::path vram_total_path =
                gpu_device_path / "mem_info_vram_total";
            std::filesystem::path vram_used_path =
                gpu_device_path / "mem_info_vram_used";
            std::filesystem::path device_id_path = gpu_device_path / "device";
            auto get_vram = [](const std::filesystem::path& p,
                               int base) -> int64_t {
              std::ifstream f(p.string());
              if (f.is_open()) {
                std::string f_str;
                std::getline(f, f_str);
                f.close();
                return std::stoll(f_str, nullptr, base);
              } else {
                std::cerr << "Error: Unable to open " << p.string()
                          << std::endl;
                return -1;
              }
            };
            auto vram_total = get_vram(vram_total_path, 10) / 1024 / 1024;
            auto vram_usage = get_vram(vram_used_path, 10) / 1024 / 1024;
            auto device_id = get_vram(device_id_path, 16);
            res[device_id] = AmdGpuUsage{.total_vram_MiB = vram_total,
                                         .used_vram_MiB = vram_usage};
          }
        } else {
          return cpp::fail("Error: Unable to open " + vendor_path.string());
        }
      }
    }
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;    
    return cpp::fail("Error: " + std::string(ex.what()));
  }

  return res;
}

#endif

// Function pointer typedefs
typedef VkResult(VKAPI_PTR* PFN_vkCreateInstance)(const VkInstanceCreateInfo*,
                                                  const VkAllocationCallbacks*,
                                                  VkInstance*);
typedef VkResult(VKAPI_PTR* PFN_vkEnumeratePhysicalDevices)(VkInstance,
                                                            uint32_t*,
                                                            VkPhysicalDevice*);
typedef void(VKAPI_PTR* PFN_vkGetPhysicalDeviceProperties)(
    VkPhysicalDevice, VkPhysicalDeviceProperties*);
typedef void(VKAPI_PTR* PFN_vkDestroyInstance)(VkInstance,
                                               const VkAllocationCallbacks*);

typedef void(VKAPI_PTR* PFN_vkGetPhysicalDeviceMemoryProperties)(
    VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceMemoryProperties* pMemoryProperties);

typedef void(VKAPI_PTR* PFN_vkGetPhysicalDeviceProperties2)(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);

typedef VkResult(VKAPI_PTR* PFN_vkEnumerateInstanceExtensionProperties)(
    const char* pLayerName, uint32_t* pPropertyCount,
    VkExtensionProperties* pProperties);

#if defined(__linux__)
inline void* GetProcAddress(void* pLibrary, const char* name) {
  return dlsym(pLibrary, name);
}

inline int FreeLibrary(void* pLibrary) {
  return dlclose(pLibrary);
}
#endif

inline cpp::result<std::vector<cortex::hw::GPU>, std::string> GetGpuInfoList() {
// Load the Vulkan library
#if defined(__APPLE__) && defined(__MACH__)
  void* vulkanLibrary = nullptr;
#elif defined(__linux__)
  void* vulkanLibrary = dlopen("libvulkan.so", RTLD_LAZY | RTLD_GLOBAL);
#else
  HMODULE vulkanLibrary = LoadLibraryW(L"vulkan-1.dll");
#endif
  if (!vulkanLibrary) {
    std::cerr << "Failed to load the Vulkan library." << std::endl;
    return cpp::fail("Failed to load the Vulkan library.");
  }

  // Get the function pointers for other Vulkan functions
  auto vkEnumerateInstanceExtensionProperties =
      reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
          GetProcAddress(vulkanLibrary,
                         "vkEnumerateInstanceExtensionProperties"));
  auto vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
      GetProcAddress(vulkanLibrary, "vkCreateInstance"));
  auto vkEnumeratePhysicalDevices =
      reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(
          GetProcAddress(vulkanLibrary, "vkEnumeratePhysicalDevices"));
  auto vkGetPhysicalDeviceProperties =
      reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
          GetProcAddress(vulkanLibrary, "vkGetPhysicalDeviceProperties"));
  auto vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(
      GetProcAddress(vulkanLibrary, "vkDestroyInstance"));
  auto vkGetPhysicalDeviceMemoryProperties =
      (PFN_vkGetPhysicalDeviceMemoryProperties)GetProcAddress(
          vulkanLibrary, "vkGetPhysicalDeviceMemoryProperties");

  auto vkGetPhysicalDeviceProperties2 =
      (PFN_vkGetPhysicalDeviceProperties2)GetProcAddress(
          vulkanLibrary, "vkGetPhysicalDeviceProperties2");

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         availableExtensions.data());

  // Create a Vulkan instance
  VkInstanceCreateInfo instanceCreateInfo = {};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  // If the extension is available, enable it
  std::vector<const char*> enabledExtensions;

  for (const auto& extension : availableExtensions) {
    enabledExtensions.push_back(extension.extensionName);
  }

  instanceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(availableExtensions.size());
  instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

  VkInstance instance;
  if (vkCreateInstance == nullptr || vkEnumeratePhysicalDevices == nullptr ||
      vkGetPhysicalDeviceProperties == nullptr ||
      vkDestroyInstance == nullptr ||
      vkGetPhysicalDeviceMemoryProperties == nullptr ||
      vkGetPhysicalDeviceProperties2 == nullptr) {
    return cpp::fail("vulkan API is missing!");
  }

  VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
  if (result != VK_SUCCESS) {
    FreeLibrary(vulkanLibrary);
    return cpp::fail("Failed to create a Vulkan instance.");
  }

  // Get the physical devices
  uint32_t physicalDeviceCount = 0;
  result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
  if (result != VK_SUCCESS) {
    vkDestroyInstance(instance, nullptr);
    FreeLibrary(vulkanLibrary);
    return cpp::fail("Failed to enumerate physical devices.");
  }
  std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
  vkEnumeratePhysicalDevices(instance, &physicalDeviceCount,
                             physicalDevices.data());

  auto uuid_to_string = [](const uint8_t* device_uuid) -> std::string {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
      if (i == 4 || i == 6 || i == 8 || i == 10) {
        ss << '-';
      }
      ss << std::setw(2) << static_cast<int>(device_uuid[i]);
    }
    return ss.str();
  };

  std::vector<cortex::hw::GPU> gpus;
#if defined(__linux__)
  auto gpus_usages =
      GetGpuUsage().value_or(std::unordered_map<int, AmdGpuUsage>{});
#elif defined(_WIN32)
  auto gpus_usages =
      GetGpuUsage().value_or(std::unordered_map<std::string, int>{});
#endif

  // Get the device properties
  size_t id = 0;
  for (const auto& physicalDevice : physicalDevices) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    VkPhysicalDeviceIDProperties deviceIDProperties = {};
    VkPhysicalDeviceProperties2 deviceProperties2 = {};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &deviceIDProperties;
    deviceIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;

    vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    int gpu_avail_MiB = 0;
    for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
      if (memoryProperties.memoryHeaps[i].flags &
          VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        gpu_avail_MiB +=
            memoryProperties.memoryHeaps[i].size / (1024ull * 1024ull);
      }
    }

    int64_t total_vram_MiB = 0;
    int64_t used_vram_MiB = 0;

#if defined(__linux__)
    total_vram_MiB = gpus_usages[deviceProperties.deviceID].total_vram_MiB;
    used_vram_MiB = gpus_usages[deviceProperties.deviceID].used_vram_MiB;
#elif defined(_WIN32)
    total_vram_MiB = gpu_avail_MiB;
    used_vram_MiB = gpus_usages[deviceProperties.deviceName];

#endif
    int free_vram_MiB =
        total_vram_MiB > used_vram_MiB ? total_vram_MiB - used_vram_MiB : 0;
    gpus.emplace_back(cortex::hw::GPU{
        .id = std::to_string(id),
        .device_id = deviceProperties.deviceID,
        .name = deviceProperties.deviceName,
        .version = std::to_string(deviceProperties.driverVersion),
        .add_info = cortex::hw::AmdAddInfo{},
        .free_vram = free_vram_MiB,
        .total_vram = total_vram_MiB,
        .uuid = uuid_to_string(deviceIDProperties.deviceUUID)});
    id++;
  }

  // Clean up
  vkDestroyInstance(instance, nullptr);
  FreeLibrary(vulkanLibrary);
  return gpus;
}
}  // namespace cortex::hw