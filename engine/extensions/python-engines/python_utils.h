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
bool IsUvInstalled();
cpp::result<void, std::string> InstallUv(
    std::shared_ptr<DownloadService>& download_service);
std::vector<std::string> BuildUvCommand(const std::string& action,
                                        const std::string& directory = "");
// cpp::result<void, std::string> UvDownloadDeps(
//     const std::filesystem::path& yaml_path);

struct PythonSubprocess {
  cortex::process::ProcessInfo proc_info;
  int port;
  uint64_t start_time;

  bool IsAlive() { return cortex::process::IsProcessAlive(proc_info); }
  bool Kill() { return cortex::process::KillProcess(proc_info); }
};
}  // namespace python_utils
