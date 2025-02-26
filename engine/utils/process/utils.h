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

namespace cortex::process {
std::string ConstructWindowsCommandLine(const std::vector<std::string>& args);

std::vector<char*> ConvertToArgv(const std::vector<std::string>& args);

pid_t SpawnProcess(const std::vector<std::string>& command,
                   const std::string stdout_file = "",
                   const std::string stderr_file = "",
                   bool wait = false);
bool IsProcessAlive(pid_t pid);
bool KillProcess(pid_t pid);

}  // namespace cortex::process
