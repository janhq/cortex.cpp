#pragma once

#if defined(_WIN32)
#include <process.h>
#include <windows.h>
using pid_t = DWORD;
#elif defined(__APPLE__) || defined(__linux__)
#include <signal.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <string>
#include <vector>
#include "utils/result.hpp"

namespace cortex::process {

// set pid to this value to signal that this pid should not be used.
constexpr pid_t PID_TERMINATED = 0;

struct ProcessInfo {
  pid_t pid;
#ifdef _WIN32
  // hJob is used to terminate process and its children.
  // hStdOut and hStdErr must be manually closed upon process termination.
  HANDLE hJob, hStdOut, hStdErr;
#endif
};

std::string ConstructWindowsCommandLine(const std::vector<std::string>& args);

std::vector<char*> ConvertToArgv(const std::vector<std::string>& args);

cpp::result<ProcessInfo, std::string> SpawnProcess(
    const std::vector<std::string>& command,
    const std::string& stdout_file = "", const std::string& stderr_file = "");
bool IsProcessAlive(ProcessInfo& proc_info);
bool WaitProcess(ProcessInfo& proc_info);
bool KillProcess(ProcessInfo& proc_info);

}  // namespace cortex::process
