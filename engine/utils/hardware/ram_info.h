#pragma once

#include <json/json.h>
#include <string>
#include "common/hardware_common.h"
#include "hwinfo/hwinfo.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/host_info.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#endif

namespace cortex::hw {

inline Memory GetMemoryInfo() {
  hwinfo::Memory m;
#if defined(__APPLE__) && defined(__MACH__)
  int64_t total_memory = 0;
  int64_t avail_memory = 0;

  size_t length = sizeof(total_memory);
  sysctlbyname("hw.memsize", &total_memory, &length, NULL, 0);

  // Get avail memory (this is a rough estimate)
  vm_size_t page_size;
  mach_msg_type_number_t count = HOST_VM_INFO_COUNT;

  vm_statistics_data_t vm_stat;
  host_page_size(mach_host_self(), &page_size);

  if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vm_stat,
                      &count) == KERN_SUCCESS) {
    avail_memory = (vm_stat.free_count + vm_stat.inactive_count) * page_size;
  }
  return Memory{.total_MiB = ByteToMiB(total_memory),
                .available_MiB = ByteToMiB(avail_memory)};
#elif defined(__linux__) || defined(_WIN32)
  return Memory{.total_MiB = ByteToMiB(m.total_Bytes()),
                .available_MiB = ByteToMiB(m.available_Bytes())};
#else
  return Memory{};
#endif
}
}  // namespace cortex::hw