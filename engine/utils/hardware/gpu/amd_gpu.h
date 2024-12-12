#pragma once
#include <stdio.h>
#include <unordered_map>
#include <string>
#include "adl_sdk.h"
#include "amd_ags.h"

namespace hardware {
typedef int (*ADL_GET_VRAM_USAGE)(ADL_CONTEXT_HANDLE context, int iAdapterIndex,
                                  int* vram_usage_in_MB);

const char* getVendorName(int vendorId) {
  switch (vendorId) {
    case 0x1002:
      return "AMD";
    case 0x8086:
      return "INTEL";
    case 0x10DE:
      return "NVIDIA";
    default:
      return "unknown";
  }
}

void PrintDisplayInfo(const AGSGPUInfo& gpuInfo) {
  for (int gpuIndex = 0; gpuIndex < gpuInfo.numDevices; gpuIndex++) {
    const AGSDeviceInfo& device = gpuInfo.devices[gpuIndex];

    printf("\n---------- Device %d%s, %s\n", gpuIndex,
           device.isPrimaryDevice ? " [primary]" : "", device.adapterString);

    printf("Vendor id:   0x%04X (%s)\n", device.vendorId,
           getVendorName(device.vendorId));
    printf("Device id:   0x%04X\n", device.deviceId);
    printf("Revision id: 0x%04X\n\n", device.revisionId);

    const char* asicFamily[] = {"unknown",  "Pre GCN",  "GCN Gen1", "GCN Gen2",
                                "GCN Gen3", "GCN Gen4", "Vega",     "RDNA",
                                "RDNA2",    "RDNA3"};

    static_assert(_countof(asicFamily) == AGSDeviceInfo::AsicFamily_Count,
                  "asic family table out of date");

    if (device.vendorId == 0x1002) {
      char wgpInfo[256] = {};
      if (device.asicFamily >= AGSDeviceInfo::AsicFamily_RDNA) {
        sprintf_s(wgpInfo, ", %d WGPs", device.numWGPs);
      }

      printf("Architecture: %s, %s%s%d CUs%s, %d ROPs\n",
             asicFamily[device.asicFamily], device.isAPU ? "(APU), " : "",
             device.isExternal ? "(External), " : "", device.numCUs, wgpInfo,
             device.numROPs);
      printf("    core clock %d MHz, memory clock %d MHz\n", device.coreClock,
             device.memoryClock);
      printf("    %.1f Tflops\n", device.teraFlops);
      printf("local memory: %d MBs (%.1f GB/s), shared memory: %d MBs\n\n",
             (int)(device.localMemoryInBytes / (1024 * 1024)),
             (float)device.memoryBandwidth / 1024.0f,
             (int)(device.sharedMemoryInBytes / (1024 * 1024)));
    }

    printf("\n");

    if (device.eyefinityEnabled) {
      printf("SLS grid is %d displays wide by %d displays tall\n",
             device.eyefinityGridWidth, device.eyefinityGridHeight);
      printf("SLS resolution is %d x %d pixels%s\n",
             device.eyefinityResolutionX, device.eyefinityResolutionY,
             device.eyefinityBezelCompensated ? ", bezel-compensated" : "");
    } else {
      printf("Eyefinity not enabled on this device\n");
    }

    printf("\n");

    for (int i = 0; i < device.numDisplays; i++) {
      const AGSDisplayInfo& display = device.displays[i];

      printf(
          "\t---------- Display %d "
          "%s----------------------------------------\n",
          i, display.isPrimaryDisplay ? "[primary]" : "---------");

      printf("\tdevice name: %s\n", display.displayDeviceName);
      printf("\tmonitor name: %s\n\n", display.name);

      printf("\tMax resolution:             %d x %d, %.1f Hz\n",
             display.maxResolutionX, display.maxResolutionY,
             display.maxRefreshRate);
      printf(
          "\tCurrent resolution:         %d x %d, Offset (%d, %d), %.1f Hz\n",
          display.currentResolution.width, display.currentResolution.height,
          display.currentResolution.offsetX, display.currentResolution.offsetY,
          display.currentRefreshRate);
      printf("\tVisible resolution:         %d x %d, Offset (%d, %d)\n\n",
             display.visibleResolution.width, display.visibleResolution.height,
             display.visibleResolution.offsetX,
             display.visibleResolution.offsetY);

      printf("\tchromaticity red:           %f, %f\n", display.chromaticityRedX,
             display.chromaticityRedY);
      printf("\tchromaticity green:         %f, %f\n",
             display.chromaticityGreenX, display.chromaticityGreenY);
      printf("\tchromaticity blue:          %f, %f\n",
             display.chromaticityBlueX, display.chromaticityBlueY);
      printf("\tchromaticity white point:   %f, %f\n\n",
             display.chromaticityWhitePointX, display.chromaticityWhitePointY);

      printf("\tluminance: [min, max, avg]  %f, %f, %f\n", display.minLuminance,
             display.maxLuminance, display.avgLuminance);

      printf("\tscreen reflectance diffuse  %f\n",
             display.screenDiffuseReflectance);
      printf("\tscreen reflectance specular %f\n\n",
             display.screenSpecularReflectance);

      if (display.HDR10)
        printf("\tHDR10 supported\n");

      if (display.dolbyVision)
        printf("\tDolby Vision supported\n");

      if (display.freesync)
        printf("\tFreesync supported\n");

      if (display.freesyncHDR)
        printf("\tFreesync HDR supported\n");

      printf("\n");

      if (display.eyefinityInGroup) {
        printf("\tEyefinity Display [%s mode] %s\n",
               display.eyefinityInPortraitMode ? "portrait" : "landscape",
               display.eyefinityPreferredDisplay ? " (preferred display)" : "");

        printf("\tGrid coord [%d, %d]\n", display.eyefinityGridCoordX,
               display.eyefinityGridCoordY);
      }

      printf("\tlogical display index: %d\n", display.logicalDisplayIndex);
      printf("\tADL adapter index: %d\n\n", display.adlAdapterIndex);

      printf("\n");
    }
  }
}

void testDriver(const char* driver, unsigned int driverToCompareAgainst) {
  AGSDriverVersionResult result =
      agsCheckDriverVersion(driver, driverToCompareAgainst);

  int major = (driverToCompareAgainst & 0xFFC00000) >> 22;
  int minor = (driverToCompareAgainst & 0x003FF000) >> 12;
  int patch = (driverToCompareAgainst & 0x00000FFF);

  if (result == AGS_SOFTWAREVERSIONCHECK_UNDEFINED) {
    printf("Driver check could not determine the driver version for %s\n",
           driver);
  } else {
    printf(
        "Driver check shows the installed %s driver is %s the %d.%d.%d "
        "required version\n",
        driver,
        result == AGS_SOFTWAREVERSIONCHECK_OK ? "newer or the same as"
                                              : "older than",
        major, minor, patch);
  }
}

void Test() {
  int displayIndex = 0;
  DISPLAY_DEVICEA displayDevice = {};
  displayDevice.cb = sizeof(displayDevice);

  AGSContext* agsContext = nullptr;
  AGSGPUInfo gpuInfo = {};
  AGSConfiguration config = {};
  if (agsInitialize(AGS_CURRENT_VERSION, &config, &agsContext, &gpuInfo) ==
      AGS_SUCCESS) {
    printf("Radeon Software Version:   %s\n", gpuInfo.radeonSoftwareVersion);
    printf("Driver Version:            %s\n", gpuInfo.driverVersion);
    printf(
        "-----------------------------------------------------------------\n");
    PrintDisplayInfo(gpuInfo);
    printf(
        "-----------------------------------------------------------------\n");

    if (agsDeInitialize(agsContext) != AGS_SUCCESS) {
      printf("Failed to cleanup AGS Library\n");
    }
  } else {
    printf("Failed to initialize AGS Library\n");
  }

  printf("\ndone\n");
}

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

int GetGpuUsage() {
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
    return 0;
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
    return 0;
  }

  // Initialize ADL. The second parameter is 1, which means:
  // retrieve adapter information only for adapters that are physically present and enabled in the system
  if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1)) {
    printf("ADL Initialization Error!\n");
    return 0;
  }

  // Obtain the number of adapters for the system
  if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&num_adapters)) {
    printf("Cannot get the number of adapters!\n");
    return 0;
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
    // std::cout << adapter_info.strAdapterName << " "
    //           << adapter_info.strDisplayName << " " << adapter_info.iVendorID
    //           << " " << adapter_active << " " << adapter_info.iDeviceNumber
    //           << std::endl;
    int vram_usage_in_MB = 0;
    if (ADL_Get_Vram_Usage) {
      ADL_Get_Vram_Usage(nullptr, i, &vram_usage_in_MB);
      // std::cout << "vram_usage_in_MB: " << vram_usage_in_MB << std::endl;
      vram_usages[adapter_info.strAdapterName] = vram_usage_in_MB;
    }
    if (adapter_active && adapter_info.iVendorID == AMDVENDORID) {
      adapter_id = adapter_info.iAdapterIndex;
      break;
    }
  }

  for(auto const& [k, v]: vram_usages) {
    std::cout << k << ": " << v << std::endl;
  }

  return 0;
}

}  // namespace hardware