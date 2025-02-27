#pragma once

#include <json/json.h>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <pdh.h>
#include <windows.h>
#pragma comment(lib, "pdh.lib")
#elif defined(__APPLE__) || defined(__MACH__)
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#else
#include <unistd.h>
#include <cmath>
#include <fstream>
#include <iterator>
#endif
#include "common/hardware_common.h"
#include "hwinfo/hwinfo.h"
#include "utils/cpuid/cpu_info.h"

namespace cortex::hw {
struct Jiffies {
  Jiffies() {
    working = -1;
    all = -1;
  }

  Jiffies(int64_t _all, int64_t _working) {
    all = _all;
    working = _working;
  }

  int64_t working;
  int64_t all;
};

struct CpuInfo {
 private:
  cortex::cpuid::CpuInfo inst;
  bool jiffies_initialized = false;

 public:
  double GetCPUUsage() {
#if defined(_WIN32)
    unsigned long long previous_total_ticks = 0;
    unsigned long long previous_idle_ticks = 0;

    auto calculate_cpu_load = [&](unsigned long long idle_ticks,
                                  unsigned long long total_ticks) {
      unsigned long long total_ticks_since_last_time =
          total_ticks - previous_total_ticks;
      unsigned long long idle_ticks_since_last_time =
          idle_ticks - previous_idle_ticks;

      float ret = 1.0f - ((total_ticks_since_last_time > 0)
                              ? ((float)idle_ticks_since_last_time) /
                                    total_ticks_since_last_time
                              : 0);

      previous_total_ticks = total_ticks;
      previous_idle_ticks = idle_ticks;
      return ret * 100;
    };

    auto file_time_to_int64 = [](const FILETIME& ft) {
      return (((unsigned long long)(ft.dwHighDateTime)) << 32) |
             ((unsigned long long)ft.dwLowDateTime);
    };

    FILETIME idle_time, kernel_time, user_time;
    float res = 0;
    constexpr const int kCount = 100;
    for (int i = 0; i < kCount; i++) {
      res += GetSystemTimes(&idle_time, &kernel_time, &user_time)
                 ? calculate_cpu_load(file_time_to_int64(idle_time),
                                      file_time_to_int64(kernel_time) +
                                          file_time_to_int64(user_time))
                 : -1.0f;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return res < 0 ? -1.0f : res / kCount;

#elif defined(__APPLE__) || defined(__MACH__)
    // macOS implementation
    host_cpu_load_info_data_t cpu_info;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

    static unsigned long long previous_total_ticks = 0;
    static unsigned long long previous_idle_ticks = 0;

    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO,
                        (host_info_t)&cpu_info, &count) == KERN_SUCCESS) {
      unsigned long long total_ticks = 0;
      for (int i = 0; i < CPU_STATE_MAX; i++) {
        total_ticks += cpu_info.cpu_ticks[i];
      }

      unsigned long long idle_ticks = cpu_info.cpu_ticks[CPU_STATE_IDLE];

      unsigned long long total_ticks_since_last_time =
          total_ticks - previous_total_ticks;
      unsigned long long idle_ticks_since_last_time =
          idle_ticks - previous_idle_ticks;

      double cpu_usage = 1.0f - ((double)idle_ticks_since_last_time /
                                 total_ticks_since_last_time);

      previous_total_ticks = total_ticks;
      previous_idle_ticks = idle_ticks;

      return cpu_usage * 100.0;
    }
    return -1.0;

#else
    if (!jiffies_initialized) {
      // Sleep 1 sec just for the start cause the usage needs to have a delta value which is depending on the unix file
      // read it's just for the init, you don't need to wait if the delta is already created ...
      std::this_thread::sleep_for(std::chrono::duration<double>(1));
      jiffies_initialized = true;
    }
    
    auto get_jiffies = [](int index) -> Jiffies {
      std::ifstream filestat("/proc/stat");
      if (!filestat.is_open()) {
        return {};
      }

      for (int i = 0; i < index; ++i) {
        if (!filestat.ignore(std::numeric_limits<std::streamsize>::max(),
                             '\n')) {
          break;
        }
      }
      std::string line;
      std::getline(filestat, line);

      std::istringstream iss(line);
      std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                       std::istream_iterator<std::string>());

      const int64_t jiffies_0 = std::stol(results[1]);
      const int64_t jiffies_1 = std::stol(results[2]);
      const int64_t jiffies_2 = std::stol(results[3]);
      const int64_t jiffies_3 = std::stol(results[4]);
      const int64_t jiffies_4 = std::stol(results[5]);
      const int64_t jiffies_5 = std::stol(results[6]);
      const int64_t jiffies_6 = std::stol(results[7]);
      const int64_t jiffies_7 = std::stol(results[8]);
      const int64_t jiffies_8 = std::stol(results[9]);
      const int64_t jiffies_9 = std::stol(results[10]);

      int64_t all = jiffies_0 + jiffies_1 + jiffies_2 + jiffies_3 + jiffies_4 +
                    jiffies_5 + jiffies_6 + jiffies_7 + jiffies_8 + jiffies_9;
      int64_t working = jiffies_0 + jiffies_1 + jiffies_2;

      return {all, working};
    };
    static Jiffies last = Jiffies();

    Jiffies current = get_jiffies(0);

    auto total_over_period = static_cast<double>(current.all - last.all);
    auto work_over_period = static_cast<double>(current.working - last.working);

    last = current;

    const double utilization = work_over_period / total_over_period;
    if (utilization < 0 || utilization > 1 || std::isnan(utilization)) {
      return -1.0;
    }
    return utilization * 100;
#endif
  }

  CPU GetCPUInfo() {
    auto res = hwinfo::getAllCPUs();
    if (res.empty())
      return CPU{};
    auto cpu = res[0];
    cortex::cpuid::CpuInfo inst;
    float usage = GetCPUUsage();
    return CPU{.cores = cpu.numPhysicalCores(),
               .arch = std::string(GetArch()),
               .model = cpu.modelName(),
               .usage = usage,
               .instructions = inst.instructions()};
  }
};
}  // namespace cortex::hw