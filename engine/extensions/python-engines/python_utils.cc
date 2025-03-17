#include "python_utils.h"
#include <filesystem>

#include "utils/archive_utils.h"
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

bool IsUvInstalled() {
  return std::filesystem::exists(GetUvPath());
}
cpp::result<void, std::string> InstallUv(
    std::shared_ptr<DownloadService>& download_service) {
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

  auto on_finished = [py_bin_path,
                      uv_version](const DownloadTask& finishedTask) {
    // try to unzip the downloaded file
    const std::string download_path = finishedTask.items[0].localPath.string();

    archive_utils::ExtractArchive(download_path, py_bin_path.string(), true);
    set_permission_utils::SetExecutePermissionsRecursive(py_bin_path);
    std::filesystem::remove(download_path);

    // install Python3.10 from Astral. this will be preferred over system
    // Python when possible.
    // NOTE: currently this will install to a user-wide directory. we can
    // install to a specific location using `--install-dir`, but later
    // invocation of `uv run` needs to have `UV_PYTHON_INSTALL_DIR` set to use
    // this Python installation.
    // we can add this once we allow passing custom env var to SpawnProcess().
    // https://docs.astral.sh/uv/reference/cli/#uv-python-install
    std::vector<std::string> command = BuildUvCommand("python");
    command.push_back("install");
    command.push_back("3.10");

    // NOTE: errors in download callback won't be propagated to caller
    auto result = cortex::process::SpawnProcess(command);
    if (result.has_error()) {
      CTL_ERR(result.error());
      return;
    }

    if (!cortex::process::WaitProcess(result.value())) {
      CTL_ERR("Process spawned but fail to wait");
      return;
    }
  };

  auto downloadTask = DownloadTask{.id = "python-uv",
                                   .type = DownloadType::Engine,
                                   .items = {DownloadItem{
                                       .id = "python-uv",
                                       .downloadUrl = url,
                                       .localPath = py_bin_path / fname,
                                   }}};

  auto add_task_result = download_service->AddTask(downloadTask, on_finished);
  if (add_task_result.has_error()) {
    return cpp::fail(add_task_result.error());
  }
  return {};
}

std::vector<std::string> BuildUvCommand(const std::string& action,
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

// cpp::result<void, std::string> UvDownloadDeps(
//     const std::filesystem::path& model_dir) {
//   if (!IsUvInstalled())
//     return cpp::fail(
//         "uv is not installed. Please run `cortex engines install python`.");

//   std::vector<std::string> command = BuildUvCommand("sync", model_dir.string());

//   // script mode. 1st argument is path to .py script
//   if (!std::filesystem::exists(model_dir / "pyproject.toml")) {
//     config::PythonModelConfig py_cfg;
//     py_cfg.ReadFromYaml((model_dir / "model.yml").string());
//     command.push_back("--script");
//     command.push_back(py_cfg.entrypoint[0]);
//   }

//   auto result = cortex::process::SpawnProcess(command);
//   if (result.has_error())
//     return cpp::fail("Fail to install Python dependencies. " + result.error());

//   if (!cortex::process::WaitProcess(result.value())) {
//     return cpp::fail("Fail to install Python dependencies.");
//   }

//   return {};
// }

}  // namespace python_utils
