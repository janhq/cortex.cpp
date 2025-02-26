#include "python_engine.h"
#include <filesystem>

#include "config/model_config.h"
#include "utils/archive_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/set_permission_utils.h"
#include "utils/system_info_utils.h"

namespace python_engine {
namespace {
constexpr const int k200OK = 200;
constexpr const int k400BadRequest = 400;
constexpr const int k409Conflict = 409;
constexpr const int k500InternalServerError = 500;
}  // namespace

cpp::result<void, std::string> DownloadUv(
    std::shared_ptr<DownloadService>& download_service) {
  const auto py_bin_path =
      file_manager_utils::GetCortexDataPath() / "python_engine" / "bin";
  std::filesystem::create_directories(py_bin_path);

  // NOTE: do we need a mechanism to update uv, or just pin uv version with cortex release?
  const std::string uv_version = "0.6.2";

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

    const pid_t pid = cortex::process::SpawnProcess(command, "", "", true);
    if (pid == -1)
      return cpp::fail("Fail to spawn process");
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

std::string GetUvPath() {
  auto system_info = system_info_utils::GetSystemInfo();
  const auto bin_name = system_info->os == kWindowsOs ? "uv.exe" : "uv";
  const auto path =
      file_manager_utils::GetCortexDataPath() / "python_engine" / "bin" / bin_name;
  return path.string();
}

// use our own cache dir so that when users delete cortexcpp/, everything is deleted.
std::string GetUvCacheDir() {
  const auto path = file_manager_utils::GetCortexDataPath() / "python_engine" /
                    "cache" / "uv";
  return path.string();
}

std::vector<std::string> BuildUvCommand(const std::string& action,
                                        const std::string& directory) {
  std::vector<std::string> command = {GetUvPath(), "--cache-dir",
                                      GetUvCacheDir()};
  if (!directory.empty()) {
    command.push_back("--directory");
    command.push_back(directory);
  }
  command.push_back(action);
  return command;
}

bool IsUvInstalled() {
  return std::filesystem::exists(GetUvPath());
}

cpp::result<void, std::string> UvDownloadDeps(
    const std::filesystem::path& model_dir) {
  if (!IsUvInstalled())
    return cpp::fail(
        "uv is not installed. Please run `cortex engines install python`.");

  std::vector<std::string> command = BuildUvCommand("sync", model_dir.string());

  // script mode. 1st argument is path to .py script
  if (!std::filesystem::exists(model_dir / "pyproject.toml")) {
    config::PythonModelConfig py_cfg;
    py_cfg.ReadFromYaml((model_dir / "model.yml").string());
    command.push_back("--script");
    command.push_back(py_cfg.entrypoint[0]);
  }

  const pid_t pid = cortex::process::SpawnProcess(command, "", "", true);
  if (pid == -1)
    return cpp::fail("Fail to install dependencies");

  return {};
}

bool PythonEngine::PythonSubprocess::IsAlive() {
  return cortex::process::IsProcessAlive(pid);
}
bool PythonEngine::PythonSubprocess::Kill() {
  return cortex::process::KillProcess(pid);
}

PythonEngine::PythonEngine() {}

PythonEngine::~PythonEngine() {
  // NOTE: what happens if we can't kill subprocess?
  std::unique_lock write_lock(mutex);
  for (auto& [model_name, py_proc] : model_process_map) {
    if (py_proc.IsAlive())
      py_proc.Kill();
  }
}

static std::pair<Json::Value, Json::Value> CreateResponse(
    const std::string& msg, int code) {

  Json::Value status, res;
  const bool has_error = code != k200OK;

  status["is_done"] = true;
  status["has_error"] = has_error;
  status["is_stream"] = false;
  status["status_code"] = code;

  if (has_error) {
    CTL_ERR(msg);
    res["error"] = msg;
  } else {
    res["status"] = msg;
  }

  return {status, res};
}

void PythonEngine::LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  if (!json_body->isMember("model") || !json_body->isMember("model_dir")) {
    auto [status, error] = CreateResponse(
        "Missing required fields: model or model_dir", k400BadRequest);
    callback(std::move(status), std::move(error));
    return;
  }

  namespace fs = std::filesystem;

  const std::string model = (*json_body)["model"].asString();
  const fs::path model_dir = (*json_body)["model_dir"].asString();

  // TODO: check if model is still alive
  {
    std::shared_lock read_lock(mutex);
    if (model_process_map.find(model) != model_process_map.end()) {
      auto [status, error] =
          CreateResponse("Model already loaded!", k409Conflict);
      callback(std::move(status), std::move(error));
      return;
    }
  }

  pid_t pid;
  try {
    config::PythonModelConfig py_cfg;
    py_cfg.ReadFromYaml((model_dir / "model.yml").string());

    if (py_cfg.entrypoint.empty()) {
      throw std::runtime_error("Missing entrypoint in model.yml");
    }

    // https://docs.astral.sh/uv/reference/cli/#uv-run
    std::vector<std::string> command =
        BuildUvCommand("run", model_dir.string());
    for (const auto& item : py_cfg.entrypoint)
      command.push_back(item);

    const std::string stdout_path = (model_dir / "stdout.txt").string();
    const std::string stderr_path = (model_dir / "stderr.txt").string();

    // create empty stdout.txt and stderr.txt for redirection
    if (!std::filesystem::exists(stdout_path))
      std::ofstream(stdout_path).flush();
    if (!std::filesystem::exists(stderr_path))
      std::ofstream(stderr_path).flush();

    // NOTE: process may start, but exits/crashes later
    // TODO: wait for a few seconds, then check if process is alive
    pid = cortex::process::SpawnProcess(command, stdout_path, stderr_path);
    if (pid == -1) {
      throw std::runtime_error("Fail to spawn process with pid -1");
    }
    const uint64_t start_time =
        std::chrono::system_clock::now().time_since_epoch() /
        std::chrono::milliseconds(1);
    std::unique_lock write_lock(mutex);
    model_process_map[model] = {pid, py_cfg.port, start_time};

  } catch (const std::exception& e) {
    auto e_msg = e.what();
    auto [status, error] = CreateResponse(e_msg, k500InternalServerError);
    callback(std::move(status), std::move(error));
    return;
  }

  auto [status, res] = CreateResponse(
      "Model loaded successfully with pid: " + std::to_string(pid), k200OK);
  callback(std::move(status), std::move(res));
}

void PythonEngine::UnloadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  if (!json_body->isMember("model")) {
    auto [status, error] =
        CreateResponse("Missing required field: model", k400BadRequest);
    callback(std::move(status), std::move(error));
    return;
  }

  const std::string model = (*json_body)["model"].asString();

  // check if model has started
  {
    std::shared_lock read_lock(mutex);
    if (model_process_map.find(model) == model_process_map.end()) {
      const std::string msg = "Model " + model + " has not been loaded yet.";
      auto [status, error] = CreateResponse(msg, k400BadRequest);
      callback(std::move(status), std::move(error));
      return;
    }
  }

  // we know that model has started
  {
    std::unique_lock write_lock(mutex);

    // check if subprocess is still alive
    if (!model_process_map[model].IsAlive()) {
      const std::string msg = "Model " + model + " stopped running.";
      auto [status, error] = CreateResponse(msg, k400BadRequest);

      // NOTE: do we need to do any other cleanup for subprocesses?
      model_process_map.erase(model);

      callback(std::move(status), std::move(error));
      return;
    }

    // subprocess is alive. we kill it here.
    if (!model_process_map[model].Kill()) {
      const std::string msg = "Unable to kill process of model " + model;
      auto [status, error] = CreateResponse(msg, k500InternalServerError);
      callback(std::move(status), std::move(error));
      return;
    }

    // NOTE: do we need to do any other cleanup for subprocesses?
    model_process_map.erase(model);
  }

  auto [status, res] = CreateResponse("Unload model successfully", k200OK);
  callback(std::move(status), std::move(res));
}

void PythonEngine::GetModelStatus(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  if (!json_body->isMember("model")) {
    auto [status, error] =
        CreateResponse("Missing required field: model", k400BadRequest);
    callback(std::move(status), std::move(error));
    return;
  }

  const std::string model = (*json_body)["model"].asString();
  Json::Value res, status;

  // check if model has started
  {
    std::shared_lock read_lock(mutex);
    if (model_process_map.find(model) == model_process_map.end()) {
      const std::string msg = "Model " + model + " has not been loaded yet.";
      auto [status, error] = CreateResponse(msg, k400BadRequest);
      callback(std::move(status), std::move(error));
      return;
    }
  }

  // we know that model has started
  {
    std::unique_lock write_lock(mutex);

    // check if subprocess is still alive
    if (!model_process_map[model].IsAlive()) {
      const std::string msg = "Model " + model + " stopped running.";
      auto [status, error] = CreateResponse(msg, k400BadRequest);

      // NOTE: do we need to do any other cleanup for subprocesses?
      model_process_map.erase(model);

      callback(std::move(status), std::move(error));
      return;
    }
  }

  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;
  callback(std::move(status), std::move(res));
}

void PythonEngine::GetModels(
    std::shared_ptr<Json::Value> jsonBody,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  Json::Value res, model_list(Json::arrayValue), status;
  {
    std::shared_lock read_lock(mutex);
    for (const auto& [model_name, py_proc] : model_process_map) {
      // TODO: check if py_proc is still alive
      Json::Value val;
      val["id"] = model_name;
      val["engine"] = kPythonEngine;
      val["start_time"] = py_proc.start_time;
      val["port"] = py_proc.port;
      val["object"] = "model";
      // TODO
      // val["ram"];
      // val["vram"];
      model_list.append(val);
    }
  }

  res["object"] = "list";
  res["data"] = model_list;

  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;

  callback(std::move(status), std::move(res));
}

cpp::result<int, std::string> PythonEngine::GetPort(const std::string& model) {
  int port;

  // check if model has started
  {
    std::shared_lock read_lock(mutex);
    if (model_process_map.find(model) == model_process_map.end()) {
      return cpp::fail("Model " + model + " has not been loaded yet.");
    }
    port = model_process_map[model].port;
  }

  // check if subprocess is still alive
  {
    std::unique_lock write_lock(mutex);
    if (!model_process_map[model].IsAlive()) {
      // NOTE: do we need to do any other cleanup for subprocesses?
      model_process_map.erase(model);
      return cpp::fail("Model " + model + " stopped running.");
    }
  }

  return port;
}

}  // namespace python_engine
