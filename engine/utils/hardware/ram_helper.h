#pragma once

#include <string>
#if defined(__APPLE__) && defined(__MACH__)
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <fstream>
#elif defined(_WIN32)
#include <psapi.h>
#include <windows.h>
#endif

namespace hardware {
struct Memory {
  uint64_t total;
  uint64_t available;
  std::string type;
};

inline Memory GetMemoryInfo() {
#if defined(__APPLE__) && defined(__MACH__)
  int64_t total_memory = 0;
  int64_t used_memory = 0;

  size_t length = sizeof(total_memory);
  sysctlbyname("hw.memsize", &total_memory, &length, NULL, 0);

  // Get used memory (this is a rough estimate)
  vm_size_t page_size;
  mach_msg_type_number_t count = HOST_VM_INFO_COUNT;

  vm_statistics_data_t vm_stat;
  host_page_size(mach_host_self(), &page_size);

  if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vm_stat,
                      &count) == KERN_SUCCESS) {
    used_memory =
        (vm_stat.active_count + vm_stat.inactive_count + vm_stat.wire_count) *
        page_size / 1024;  // Convert to KB
  }
  return Memory{.total = total_memory, .available = total_memory - used_memory};
#elif defined(__linux__)
  std::ifstream meminfo("/proc/meminfo");
  std::string line;
  uint64_t total_memory = 0;
  uint64_t free_memory = 0;
  while (std::getline(meminfo, line)) {
    if (line.find("MemTotal:") == 0) {
      sscanf(line.c_str(), "MemTotal: %ld kB", &total_memory);
    }
    if (line.find("MemAvailable:") == 0) {
      sscanf(line.c_str(), "MemAvailable: %ld kB", &free_memory);
    }
  }

  return Memory{.total = total_memory, .available = free_memory};
#elif defined(_WIN32)
  PROCESS_MEMORY_COUNTERS pmc;
  if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
    // Get total physical memory
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return Memory{
        .total = statex.ullTotalPhys / 1024,
        .available = (statex.ullTotalPhys - pmc.WorkingSetSize) / 1024};
  }
  return Memory{};
#else
  return Memory{};
#endif
}
}  // namespace hardware