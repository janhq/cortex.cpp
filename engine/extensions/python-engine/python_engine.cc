#include "python_engine.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

namespace python_engine {
namespace {
constexpr const int k200OK = 200;
constexpr const int k400BadRequest = 400;
constexpr const int k409Conflict = 409;
constexpr const int k500InternalServerError = 500;
constexpr const int kFileLoggerOption = 0;
}  // namespace

cpp::result<void, std::string> DownloadUv(std::shared_ptr<DownloadService>& download_service) {
  const std::string py_bin_path = file_manager_utils::GetCortexDataPath() / "python_engine" / "bin";
  std::filesystem::create_directories(py_bin_path);

  const std::string uv_version = "0.5.31";

  // NOTE: only works on MacOS and Linux
  auto on_finished = [py_bin_path, uv_version](const DownloadTask& finishedTask) {
    // try to unzip the downloaded file
    const std::string installer_path = finishedTask.items[0].localPath.string();
    CTL_INF("UV install script path: " << installer_path);
    CTL_INF("Version: " << uv_version);

    // https://docs.astral.sh/uv/configuration/installer/
    // TODO: move env var mod logic to SpawnProcess()
    // using env to set env vars
    // should we download from here instead? https://github.com/astral-sh/uv/releases
    std::vector<std::string> command{"env",
                                     "UV_UNMANAGED_INSTALL=" + py_bin_path,
                                     "sh",
                                     installer_path,
                                     "-q"};
    const auto pid = cortex::process::SpawnProcess(command);
    if (pid == -1) {
      CTL_ERR("Failed to install uv");
    }
    // wait for subprocess to finish
    // TODO: need to check return status if successful
    waitpid(pid, NULL, 0);
    std::filesystem::remove(installer_path);
  };

  const std::string url = "https://astral.sh/uv/" + uv_version + "/install.sh";
  auto downloadTask =
    DownloadTask{.id = "uv",
                 .type = DownloadType::Engine,
                 .items = {DownloadItem{
                      .id = "uv",
                      .downloadUrl = url,
                      .localPath = py_bin_path + "/install.sh",
                  }}};

  auto add_task_result = download_service->AddTask(downloadTask, on_finished);
  if (add_task_result.has_error()) {
    return cpp::fail(add_task_result.error());
  }
  return {};
}

std::string GetUvPath() {
  return file_manager_utils::GetCortexDataPath() / "python_engine" / "bin" / "uv";
}
bool IsUvInstalled() {
  return std::filesystem::exists(GetUvPath());
}

bool PythonEngine::PythonSubprocess::IsAlive() {
  return cortex::process::IsProcessAlive(pid);
}
bool PythonEngine::PythonSubprocess::Kill() {
  return cortex::process::KillProcess(pid);
}

PythonEngine::PythonEngine() : q_(4 /*n_parallel*/, "python_engine") {}

PythonEngine::~PythonEngine() {
  curl_global_cleanup();
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
  }
  else {
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
      auto [status, error] = CreateResponse(
        "Model already loaded!", k409Conflict);
      callback(std::move(status), std::move(error));
      return;
    }
  }

  pid_t pid;
  try {
    config::PythonModelConfig py_cfg;
    py_cfg.ReadFromYaml(model_dir / "model.yml");

    // NOTE: model_dir / entrypoint assumes a Python script
    // TODO: figure out if we can support arbitrary CLI (but still launch by uv)
    std::vector<std::string> command{GetUvPath(), "run", model_dir / py_cfg.entrypoint};
    for (const auto& item : py_cfg.extra_args)
      command.push_back(item);

    const std::string stdout_path = model_dir / "stdout.txt";
    const std::string stderr_path = model_dir / "stderr.txt";

    // create empty stdout.txt and stderr.txt for redirection
    if (!std::filesystem::exists(stdout_path)) std::ofstream(stdout_path).flush();
    if (!std::filesystem::exists(stderr_path)) std::ofstream(stderr_path).flush();

    // TODO: what happens if the process starts, but exits?
    pid = cortex::process::SpawnProcess(command, stdout_path, stderr_path);
    if (pid == -1) {
      throw std::runtime_error("Fail to spawn process with pid -1");
    }
    std::unique_lock write_lock(mutex);
    model_process_map[model] = {pid, py_cfg.port};

  } catch (const std::exception& e) {
    auto e_msg = e.what();
    auto [status, error] = CreateResponse(e_msg, k500InternalServerError);
    callback(std::move(status), std::move(error));
    return;
  }

  auto [status, res] = CreateResponse(
    "Model loaded successfully with pid: " + std::to_string(pid),
    k200OK);
  callback(std::move(status), std::move(res));
}

void PythonEngine::UnloadModel(
  std::shared_ptr<Json::Value> json_body,
  std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  if (!json_body->isMember("model")) {
    auto [status, error] = CreateResponse("Missing required field: model", k400BadRequest);
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
    auto [status, error] = CreateResponse("Missing required field: model", k400BadRequest);
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
      val["port"] = py_proc.port;
      val["object"] = "model";
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

void PythonEngine::HandleRequest(
  const std::string& model,
  const std::vector<std::string>& path_parts,
  std::shared_ptr<Json::Value> json_body,
  std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  assert(false && "Not implemented");
  // get port
}

}  // namespace python_engine
