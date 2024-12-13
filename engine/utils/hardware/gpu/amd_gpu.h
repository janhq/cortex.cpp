#pragma once
#include <stdio.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "adl_sdk.h"
#include "amd_ags.h"
#include "common/hardware_common.h"
#include "utils/result.hpp"
#include "vulkan/vulkan.h"

namespace hardware {
////////
#define AMDVENDORID (1002)
#define ADL_WARNING_NO_DATA -100

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int (*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int (*ADL_MAIN_CONTROL_DESTROY)();
typedef int (*ADL_ADAPTER_NUMBEROFADAPTERS_GET)(int*);
typedef int (*ADL_ADAPTER_ADAPTERINFO_GET)(LPAdapterInfo, int);
typedef int (*ADL_ADAPTER_ACTIVE_GET)(int, int*);
typedef int (*ADL_OVERDRIVE_CAPS)(int iAdapterIndex, int* iSupported,
                                  int* iEnabled, int* iVersion);
typedef int (*ADL_GET_VRAM_USAGE)(ADL_CONTEXT_HANDLE context, int iAdapterIndex,
                                  int* vram_usage_in_MB);

// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc(int iSize) {
  void* lpBuffer = malloc(iSize);
  return lpBuffer;
}

// Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free(void** lpBuffer) {
  if (NULL != *lpBuffer) {
    free(*lpBuffer);
    *lpBuffer = NULL;
  }
}

#if defined(LINUX)
// equivalent functions in linux
void* GetProcAddress(void* pLibrary, const char* name) {
  return dlsym(pLibrary, name);
}
#endif

inline cpp::result<std::unordered_map<std::string, int>, std::string>
GetGpuUsage() {
#if defined(LINUX)
  void* hDLL;  // Handle to .so library
#else
  HINSTANCE hDLL;  // Handle to DLL
#endif

  LPAdapterInfo lpAdapterInfo = NULL;
  int i;
  int num_adapters = 0;

#if defined(LINUX)
  hDLL = dlopen("libatiadlxx.so", RTLD_LAZY | RTLD_GLOBAL);
#else
  hDLL = LoadLibrary(L"atiadlxx.dll");
  if (hDLL == NULL)
    // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
    // Try to load the 32 bit library (atiadlxy.dll) instead
    hDLL = LoadLibrary(L"atiadlxy.dll");
#endif

  if (NULL == hDLL) {
    printf("ADL library not found!\n");
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

  auto ADL_Get_Vram_Usage =
      (ADL_GET_VRAM_USAGE)GetProcAddress(hDLL, "ADL2_Adapter_VRAMUsage_Get");

  if (NULL == ADL_Main_Control_Create || NULL == ADL_Main_Control_Destroy ||
      NULL == ADL_Adapter_NumberOfAdapters_Get ||
      NULL == ADL_Adapter_AdapterInfo_Get || NULL == ADL_Adapter_Active_Get ||
      nullptr == ADL_Get_Vram_Usage) {
    printf("ADL's API is missing!\n");
    return cpp::fail("ADL's API is missing!");
  }

  // Initialize ADL. The second parameter is 1, which means:
  // retrieve adapter information only for adapters that are physically present and enabled in the system
  if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1)) {
    printf("ADL Initialization Error!\n");
    return cpp::fail("ADL Initialization Error!");
  }

  // Obtain the number of adapters for the system
  if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&num_adapters)) {
    printf("Cannot get the number of adapters!\n");
    return cpp::fail("Cannot get the number of adapters!");
  }
  std::cout << "num_adapters: " << num_adapters << std::endl;

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
    int vram_usage_in_MB = 0;
    if (ADL_Get_Vram_Usage) {
      ADL_Get_Vram_Usage(nullptr, i, &vram_usage_in_MB);
      vram_usages[adapter_info.strAdapterName] = vram_usage_in_MB;
    }
  }

  for (auto const& [k, v] : vram_usages) {
    std::cout << k << ": " << v << std::endl;
  }

  return vram_usages;
}

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

inline cpp::result<std::vector<cortex::hw::GPU>, std::string> GetGpuListInfo() {
  // Load the Vulkan library
  HMODULE vulkanLibrary = LoadLibraryW(L"vulkan-1.dll");
  std::cout << "1" << std::endl;
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
    std::cout << "vulkan API is missing!" << std::endl;
    return cpp::fail("vulkan API is missing!");
  }

  VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
  if (result != VK_SUCCESS) {
    std::cerr << "Failed to create a Vulkan instance." << std::endl;
    FreeLibrary(vulkanLibrary);
    return cpp::fail("Failed to create a Vulkan instance.");
  }

  // Get the physical devices
  uint32_t physicalDeviceCount = 0;
  result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
  if (result != VK_SUCCESS) {
    std::cerr << "Failed to enumerate physical devices." << std::endl;
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
  auto gpus_usages =
      GetGpuUsage().value_or(std::unordered_map<std::string, int>{});

  // Get the device properties
  size_t id = 0;
  for (const auto& physicalDevice : physicalDevices) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    std::cout << "GPU Name: " << deviceProperties.deviceName << std::endl;
    std::cout << "Vendor ID: " << deviceProperties.vendorID << std::endl;
    std::cout << "Device ID: " << deviceProperties.deviceID << std::endl;
    std::cout << "API Version: "
              << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
              << VK_VERSION_MINOR(deviceProperties.apiVersion) << "."
              << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;

    VkPhysicalDeviceIDProperties deviceIDProperties = {};
    VkPhysicalDeviceProperties2 deviceProperties2 = {};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &deviceIDProperties;
    deviceIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;

    vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

    // The Device UUID is stored in the deviceUUID member
    // const uint8_t* deviceUUID = deviceIDProperties.deviceUUID;

    std::cout << std::endl;

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

    std::cout << "GPU Memory Size: " << gpu_avail_MiB << " MiB" << std::endl;
    int64_t usage_vram_MiB = gpus_usages[deviceProperties.deviceName];
    gpus.emplace_back(
        cortex::hw::GPU{.id = std::to_string(id),
                        .device_id = deviceProperties.deviceID,
                        .name = deviceProperties.deviceName,
                        .version = "",
                        .add_info = cortex::hw::AmdAddInfo{},
                        .free_vram = gpu_avail_MiB - usage_vram_MiB,
                        .total_vram = gpu_avail_MiB,
                        .uuid = uuid_to_string(deviceIDProperties.deviceUUID)});
    std::cout << gpus.back().uuid << " " << gpus.back().free_vram << std::endl;
    id++;
  }

  // Clean up
  vkDestroyInstance(instance, nullptr);
  FreeLibrary(vulkanLibrary);
  std::cout << "Done" << std::endl;
  return gpus;
}

}  // namespace hardware