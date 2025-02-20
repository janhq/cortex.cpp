#pragma once
#include <chrono>
#include <iostream>
#include <sstream>
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
#include <fstream>
#endif
#include "utils/logging_utils.h"

namespace cortex::hw {
inline double GetCPUUsage() {
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
  // Linux implementation
  std::vector<unsigned long long> last_total_user, last_total_user_low,
      last_total_sys, last_total_idle;

  std::vector<unsigned long long> total_user, total_user_low, total_sys,
      total_idle;

  std::ifstream stat_file("/proc/stat");
  if (stat_file.is_open()) {
    std::string line;
    int cpu_count = 0;
    double total_cpu_percentage = 0.0;

    while (std::getline(stat_file, line)) {
      if (line.substr(0, 3) != "cpu")
        break;  // We only want lines that start with "cpu"

      cpu_count++;
      std::vector<unsigned long long> values;
      std::istringstream iss(line);
      std::string cpu;
      iss >> cpu;
      unsigned long long value;
      while (iss >> value) {
        values.push_back(value);
      }

      if (values.size() < 4)
        continue;

      total_user.push_back(values[0]);
      total_user_low.push_back(values[1]);
      total_sys.push_back(values[2]);
      total_idle.push_back(values[3]);

      if (last_total_user.size() < cpu_count) {
        last_total_user.push_back(0);
        last_total_user_low.push_back(0);
        last_total_sys.push_back(0);
        last_total_idle.push_back(0);
      }

      unsigned long long total =
          (total_user[cpu_count - 1] - last_total_user[cpu_count - 1]) +
          (total_user_low[cpu_count - 1] - last_total_user_low[cpu_count - 1]) +
          (total_sys[cpu_count - 1] - last_total_sys[cpu_count - 1]);
      double percent = total;
      total += (total_idle[cpu_count - 1] - last_total_idle[cpu_count - 1]);
      percent /= total;
      percent *= 100;

      total_cpu_percentage += percent;

      last_total_user[cpu_count - 1] = total_user[cpu_count - 1];
      last_total_user_low[cpu_count - 1] = total_user_low[cpu_count - 1];
      last_total_sys[cpu_count - 1] = total_sys[cpu_count - 1];
      last_total_idle[cpu_count - 1] = total_idle[cpu_count - 1];
    }
    stat_file.close();

    if (cpu_count > 0) {
      return total_cpu_percentage / cpu_count;  // Return average CPU usage
    }
  }
  return -1.0;
#endif
}
}  // namespace cortex::hw