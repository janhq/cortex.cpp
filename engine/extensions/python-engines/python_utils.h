#pragma once

#include <filesystem>
#include <string>

#include "services/download_service.h"
#include "utils/process/utils.h"

namespace python_utils {

// paths
std::filesystem::path GetPythonEnginesPath();
std::filesystem::path GetEnvsPath();
std::filesystem::path GetUvPath();

// UV-related functions
bool UvIsInstalled();
cpp::result<void, std::string> UvInstall();
std::vector<std::string> UvBuildCommand(const std::string& action,
                                        const std::string& directory = "");
bool UvCleanCache();

struct PythonSubprocess {
  cortex::process::ProcessInfo proc_info;
  int port;
  uint64_t start_time;

  bool IsAlive() { return cortex::process::IsProcessAlive(proc_info); }
  bool Kill() { return cortex::process::KillProcess(proc_info); }
};
}  // namespace python_utils
