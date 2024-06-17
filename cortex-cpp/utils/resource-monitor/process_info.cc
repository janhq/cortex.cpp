

#include "process_info.h"
/*main info got from msdn (winapi) and https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process */

#if defined(WIN32) | defined(_WIN64)

#include "Windows.h"

#include <Psapi.h>
#include <processthreadsapi.h>
#include <xutility>
namespace cortex::rsrc_mon {

struct ProcessInfo::Impl : public ProcessInfoI {

 public:
  Impl()
      : pid_process_(GetCurrentProcessId()) /*set own process id*/,
        process_handle_(OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false,
            pid_process_)) /*open process with handle - for retrieve info*/,
        num_processors_(
            getProcessorNumber()) /*count processors - need for cpu*/
  {}

  ~Impl() override { CloseHandle(process_handle_); }

  double GetCpuUsage() override {
    FILETIME now;
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    int64_t system_time;
    int64_t time;
    int64_t system_time_delta;
    int64_t time_delta;

    double cpu = -1;

    if (!process_handle_)
      return -1;

    GetSystemTimeAsFileTime(&now);

    if (!GetProcessTimes(process_handle_, &creation_time, &exit_time,
                         &kernel_time, &user_time)) {
      return -1;
    }
    system_time = (fileTime2Utc(&kernel_time) + fileTime2Utc(&user_time)) /
                  num_processors_;
    time = fileTime2Utc(&now);

    if ((last_system_time_ == 0) || (last_time_ == 0)) {
      last_system_time_ = system_time;
      last_time_ = time;
      return -1;
    }

    system_time_delta = system_time - last_system_time_;
    time_delta = time - last_time_;

    cpu = (double)system_time_delta * 100 / (double)time_delta;
    last_system_time_ = system_time;
    last_time_ = time;
    return cpu;
  }

  int64_t GetMemoryUsage() override {
    int64_t memory_usage = 0;

    PROCESS_MEMORY_COUNTERS_EX pmc;

    if (GetProcessMemoryInfo(process_handle_, (PROCESS_MEMORY_COUNTERS*)&pmc,
                             sizeof(pmc))) {
      memory_usage = std::move(pmc.PrivateUsage);
    } else {
      memory_usage = -1; /*can't retrieve info*/
    }

    return memory_usage;
  }

 private:
  static int64_t fileTime2Utc(const FILETIME* ftime) {
    LARGE_INTEGER li;
    li.LowPart = ftime->dwLowDateTime;
    li.HighPart = ftime->dwHighDateTime;
    return li.QuadPart;
  }

  static int getProcessorNumber() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
  }

 private:
  DWORD pid_process_;
  HANDLE process_handle_;

  int32_t num_processors_{0};
  int64_t last_time_{0};
  int64_t last_system_time_{0};
};
/*WIN32*/
#elif __linux__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "sys/times.h"
#include "sys/vtimes.h"
namespace cortex::rsrc_mon {
struct ProcessInfo::Impl : public ProcessInfoI {

 public:
  Impl() { init(); }

  ~Impl() override {}

  double GetCpuUsage() override {
    struct tms timeSample;
    clock_t now;
    double percent;

    now = times(&timeSample);
    if (now <= last_cpu_ || timeSample.tms_stime < last_sys_cpu_ ||
        timeSample.tms_utime < last_user_cpu_) {
      // Overflow detection. Just skip this value.
      percent = -1.0;
    } else {
      percent = (timeSample.tms_stime - last_sys_cpu_) +
                (timeSample.tms_utime - last_user_cpu_);
      percent /= (now - last_cpu_);
      percent /= num_processors_;
      percent *= 100;
    }
    last_cpu_ = now;
    last_sys_cpu_ = timeSample.tms_stime;
    last_user_cpu_ = timeSample.tms_utime;

    return percent;
  }

  int64_t GetMemoryUsage() override {
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL) {
      if (strncmp(line, "VmRSS:", 6) == 0) {
        result = parseLine(line);
        break;
      }
    }
    fclose(file);
    return result * 1024;
  }

 private:
  int parseLine(char* line) {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p < '0' || *p > '9')
      p++;
    line[i - 3] = '\0';
    i = atoi(p);
    return i;
  }

  void init() {
    FILE* file;
    struct tms timeSample;
    char line[128];

    last_cpu_ = times(&timeSample);
    last_sys_cpu_ = timeSample.tms_stime;
    last_user_cpu_ = timeSample.tms_utime;

    file = fopen("/proc/cpuinfo", "r");
    num_processors_ = 0;
    while (fgets(line, 128, file) != NULL) {
      if (strncmp(line, "processor", 9) == 0)
        num_processors_++;
    }
    fclose(file);
  }

 private:
  clock_t last_cpu_{0};
  clock_t last_sys_cpu_{0};
  clock_t last_user_cpu_{0};
  int num_processors_{0};
};

#else /*MacOS*/  // TODO(sang)
#include <mach/mach.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "sys/times.h"
namespace cortex::rsrc_mon {

struct ProcessInfo::Impl : public ProcessInfoI {

 public:
  Impl() { init(); }

  ~Impl() override {}

  double GetCpuUsage() override {
    struct tms timeSample;
    clock_t now;
    double percent;

    now = times(&timeSample);
    if (now <= last_cpu_ || timeSample.tms_stime < last_sys_cpu_ ||
        timeSample.tms_utime < last_user_cpu_) {
      // Overflow detection. Just skip this value.
      percent = -1.0;
    } else {
      percent = (timeSample.tms_stime - last_sys_cpu_) +
                (timeSample.tms_utime - last_user_cpu_);
      percent /= (now - last_cpu_);
      percent /= num_processors_;
      percent *= 100;
    }
    last_cpu_ = now;
    last_sys_cpu_ = timeSample.tms_stime;
    last_user_cpu_ = timeSample.tms_utime;

    return percent;
  }

  int64_t GetMemoryUsage() override {
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (KERN_SUCCESS != task_info(mach_task_self(), TASK_BASIC_INFO,
                                  (task_info_t)&t_info, &t_info_count)) {
      return -1;
    }
    return t_info.resident_size;
  }

 private:
  void init() {
    int nm[2];
    size_t len = 4;
    uint32_t count;

    nm[0] = CTL_HW;
    nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if (count < 1) {
      nm[1] = HW_NCPU;
      sysctl(nm, 2, &count, &len, NULL, 0);
      if (count < 1) {
        count = 1;
      }
    }
    num_processors_ = count;
  }

 private:
  clock_t last_cpu_{0};
  clock_t last_sys_cpu_{0};
  clock_t last_user_cpu_{0};
  int num_processors_{0};
};
#endif

ProcessInfo::ProcessInfo() {
  impl_ = new Impl();
}

ProcessInfo::~ProcessInfo() {
  delete impl_;
}

double ProcessInfo::GetCpuUsage() {
  return impl_->GetCpuUsage();
}

int64_t ProcessInfo::GetMemoryUsage() {
  return impl_->GetMemoryUsage();
}
}