

#include "system_info.h"

/*main info got from msdn and https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process */

#if defined(WIN32) | defined(_WIN64)

#pragma comment(lib, "pdh.lib")
#include <Pdh.h>
#include <Windows.h>

namespace cortex::rsrc_mon {    
struct SystemInfo::Impl : public SystemInfoI {
 public:
  Impl() { Init(); }

  ~Impl() override = default;

  int64_t GetTotalMemory() override {
    MEMORYSTATUSEX mem_info;
    mem_info.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&mem_info);
    DWORDLONG totalPhysMem = mem_info.ullTotalPhys;

    return totalPhysMem;
  }

  int64_t GetTotalUsageMemory() override {
    MEMORYSTATUSEX mem_info;
    mem_info.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&mem_info);
    DWORDLONG physMemUsed = mem_info.ullTotalPhys - mem_info.ullAvailPhys;

    return physMemUsed;
  }

  double GetCpuTotalUsage() override {
    PDH_FMT_COUNTERVALUE counterVal;

    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    return counterVal.doubleValue;
  }

 private:
  void Init() {
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    // You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU values with PdhGetFormattedCounterArray()
    PdhAddEnglishCounter(cpuQuery,
                         LPSTR("\\Processor(_Total)\\% Processor Time"), NULL,
                         &cpuTotal);
    PdhCollectQueryData(cpuQuery);
  }

 private:
  PDH_HQUERY cpuQuery;
  PDH_HCOUNTER cpuTotal;
};
/*WIN32*/
#elif __linux__
/*memory*/
#include "sys/sysinfo.h"
#include "sys/times.h"
#include "sys/types.h"
#include "sys/vtimes.h"

/*cpu*/
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace cortex::rsrc_mon {
struct SystemInfo::Impl : public SystemInfoI {
 public:
  Impl() = default;

  ~Impl() override = default;

  int64_t GetTotalMemory() override {
    struct sysinfo mem_info;

    sysinfo(&mem_info);
    long long totalPhysMem = mem_info.totalram;
    //Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= mem_info.mem_unit;

    return totalPhysMem;
  }

  int64_t GetTotalUsageMemory() override {
    struct sysinfo mem_info;

    sysinfo(&mem_info);
    long long physMemUsed = mem_info.totalram - mem_info.freeram;
    //Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= mem_info.mem_unit;

    return physMemUsed;
  }

  double GetCpuTotalUsage() override {
    double percent;
    FILE* file;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

    file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
           &totalSys, &totalIdle);
    fclose(file);

    if (totalUser < last_total_user_ || totalUserLow < last_total_user_low_ ||
        totalSys < last_total_sys_ || totalIdle < last_total_idle_) {
      //Overflow detection. Just skip this value.
      percent = -1.0;
    } else {
      total = (totalUser - last_total_user_) +
              (totalUserLow - last_total_user_low_) +
              (totalSys - last_total_sys_);
      percent = total;
      total += (totalIdle - last_total_idle_);
      percent /= total;
      percent *= 100;
    }

    last_total_user_ = totalUser;
    last_total_user_low_ = totalUserLow;
    last_total_sys_ = totalSys;
    last_total_idle_ = totalIdle;

    return percent;
  }

 private:
  void Init() {
    FILE* file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &last_total_user_,
           &last_total_user_low_, &last_total_sys_, &last_total_idle_);
    fclose(file);
  }

 private:
  unsigned long long last_total_user_;
  unsigned long long last_total_user_low_;
  unsigned long long last_total_sys_;
  unsigned long long last_total_idle_;
};

#else /*MacOS*/
#include <sys/mount.h>
#include <sys/param.h>
#include "sys/times.h"

#include <mach/mach_error.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/mach_types.h>
#include <mach/vm_map.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>
#include <sys/types.h>

/*cpu*/
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace cortex::rsrc_mon {

struct SystemInfo::Impl : public SystemInfoI {
 public:
  Impl() = default;

  ~Impl() override = default;

  int64_t GetTotalMemory() override {
    int mib[2];
    int64_t physical_memory;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    auto length = sizeof(int64_t);
    sysctl(mib, 2, &physical_memory, &length, NULL, 0);
    return physical_memory;
  }

  int64_t GetTotalUsageMemory() override {
    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics64_data_t vm_stats;

    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
        KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
                                          (host_info64_t)&vm_stats, &count)) {
      long long free_memory = (int64_t)vm_stats.free_count * (int64_t)page_size;

      long long used_memory =
          ((int64_t)vm_stats.active_count + (int64_t)vm_stats.inactive_count +
           (int64_t)vm_stats.wire_count) *
          (int64_t)page_size;
      printf("free memory: %lld\nused memory: %lld\n", free_memory,
             used_memory);
      return used_memory;
    }
    return -1;
  }

  // Returns 1.0f for "CPU fully pinned", 0.0f for "CPU idle", or somewhere in between
  // You'll need to call this at regular intervals, since it measures the load between
  // the previous call and the current one.
  double GetCpuTotalUsage() override {
    host_cpu_load_info_data_t cpu_info;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO,
                        (host_info_t)&cpu_info, &count) == KERN_SUCCESS) {
      unsigned long long total_ticks = 0;
      for (int i = 0; i < CPU_STATE_MAX; i++)
        total_ticks += cpu_info.cpu_ticks[i];
      return CalculateCPULoad(cpu_info.cpu_ticks[CPU_STATE_IDLE], total_ticks);
    } else
      return -1.0f;
  }

 private:
  float CalculateCPULoad(unsigned long long idle_ticks,
                         unsigned long long total_ticks) {
    unsigned long long total_ticks_since_last_time =
        total_ticks - previous_total_ticks_;
    unsigned long long idle_ticks_since_last_time =
        idle_ticks - previous_idle_ticks_;
    float ret = 1.0f - ((total_ticks_since_last_time > 0)
                            ? ((float)idle_ticks_since_last_time) /
                                  total_ticks_since_last_time
                            : 0);
    previous_total_ticks_ = total_ticks;
    previous_idle_ticks_ = idle_ticks;
    return ret;
  }

 private:
  unsigned long long previous_total_ticks_ = 0;
  unsigned long long previous_idle_ticks_ = 0;
};
#endif

SystemInfo::SystemInfo() {
  impl_ = new Impl();
}

SystemInfo::~SystemInfo() noexcept {
  delete impl_;
}

int64_t SystemInfo::GetTotalMemory() {
  return impl_->GetTotalMemory();
}

int64_t SystemInfo::GetTotalUsageMemory() {
  return impl_->GetTotalUsageMemory();
}

double SystemInfo::GetCpuTotalUsage() {
  return impl_->GetCpuTotalUsage();
}
}  // namespace cortex::rsrc_mon