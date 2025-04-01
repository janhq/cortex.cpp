#include "python_utils.h"
#include <filesystem>

#include "utils/archive_utils.h"
#include "utils/curl_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/set_permission_utils.h"
#include "utils/system_info_utils.h"

namespace python_utils {

std::filesystem::path GetPythonEnginesPath() {
  return file_manager_utils::GetCortexDataPath() / "python_engines";
}
std::filesystem::path GetEnvsPath() {
  return GetPythonEnginesPath() / "envs";
}
std::filesystem::path GetUvPath() {
  auto system_info = system_info_utils::GetSystemInfo();
  const auto bin_name = system_info->os == kWindowsOs ? "uv.exe" : "uv";
  return GetPythonEnginesPath() / "bin" / bin_name;
}
bool UvCleanCache() {
  auto cmd = UvBuildCommand("cache");
  cmd.push_back("clean");
  auto result = cortex::process::SpawnProcess(cmd);
  if (result.has_error()) {
    CTL_INF(result.error());
    return false;
  }
  return cortex::process::WaitProcess(result.value());
}

bool UvIsInstalled() {
  return std::filesystem::exists(GetUvPath());
}
cpp::result<void, std::string> UvInstall() {
  const auto py_bin_path = GetPythonEnginesPath() / "bin";
  std::filesystem::create_directories(py_bin_path);

  // NOTE: do we need a mechanism to update uv, or just pin uv version with cortex release?
  const std::string uv_version = "0.6.3";

  // build download url based on system info
  std::stringstream fname_stream;
  fname_stream << "uv-";

  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info->arch == "amd64")
    fname_stream << "x86_64";
  else if (system_info->arch == "arm64")
    fname_stream << "aarch64";

  // NOTE: there is also a musl linux version
  if (system_info->os == kMacOs)
    fname_stream << "-apple-darwin.tar.gz";
  else if (system_info->os == kWindowsOs)
    fname_stream << "-pc-windows-msvc.zip";
  else if (system_info->os == kLinuxOs)
    fname_stream << "-unknown-linux-gnu.tar.gz";

  const std::string fname = fname_stream.str();
  const std::string base_url =
      "https://github.com/astral-sh/uv/releases/download/";

  std::stringstream url_stream;
  url_stream << base_url << uv_version << "/" << fname;
  const std::string url = url_stream.str();
  CTL_INF("Download uv from " << url);

  const auto save_path = py_bin_path / fname;
  auto res = curl_utils::SimpleDownload(url, save_path.string());
  if (res.has_error())
    return res;

  archive_utils::ExtractArchive(save_path, py_bin_path.string(), true);
  set_permission_utils::SetExecutePermissionsRecursive(py_bin_path);
  std::filesystem::remove(save_path);

  // install Python3.10 from Astral. this will be preferred over system
  // Python when possible.
  // NOTE: currently this will install to a user-wide directory. we can
  // install to a specific location using `--install-dir`, but later
  // invocation of `uv run` needs to have `UV_PYTHON_INSTALL_DIR` set to use
  // this Python installation.
  // we can add this once we allow passing custom env var to SpawnProcess().
  // https://docs.astral.sh/uv/reference/cli/#uv-python-install
  std::vector<std::string> command = UvBuildCommand("python");
  command.push_back("install");
  command.push_back("3.10");

  auto result = cortex::process::SpawnProcess(command);
  if (result.has_error())
    return cpp::fail(result.error());

  if (!cortex::process::WaitProcess(result.value())) {
    const auto msg = "Process spawned but fail to wait";
    CTL_ERR(msg);
    return cpp::fail(msg);
  }

  return {};
}

std::vector<std::string> UvBuildCommand(const std::string& action,
                                        const std::string& directory) {
  // use our own cache dir so that when users delete cortexcpp/, everything is deleted.
  const auto cache_dir = GetPythonEnginesPath() / "cache" / "uv";
  std::vector<std::string> command = {GetUvPath().string(), "--cache-dir",
                                      cache_dir.string()};
  if (!directory.empty()) {
    command.push_back("--directory");
    command.push_back(directory);
  }
  command.push_back(action);
  return command;
}

}  // namespace python_utils
