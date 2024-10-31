#pragma once

#include <json/json.h>
#include <string>

#include "hwinfo/hwinfo.h"
#if defined(__APPLE__) && defined(__MACH__)
#include <mach/host_info.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#endif

namespace hardware {
struct Memory {
  int64_t total;
  int64_t available;
  std::string type;
};

inline Json::Value ToJson(const Memory& m) {
  Json::Value res;
  res["total"] = m.total;
  res["available"] = m.available;
  res["type"] = m.type;
  return res;
}

inline Memory GetMemoryInfo() {
  hwinfo::Memory m;
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
#elif defined(__linux__) || defined(_WIN32)
  return Memory{.total = m.total_Bytes(), .available = m.available_Bytes()};
#else
  return Memory{};
#endif
}
}  // namespace hardware