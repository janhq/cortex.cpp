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

size_t StreamWriteCallback(char* ptr, size_t size, size_t nmemb,
                           void* userdata) {
  auto* context = static_cast<StreamContext*>(userdata);
  std::string chunk(ptr, size * nmemb);

  context->buffer += chunk;

  // Process complete lines
  size_t pos;
  while ((pos = context->buffer.find('\n')) != std::string::npos) {
    std::string line = context->buffer.substr(0, pos);
    context->buffer = context->buffer.substr(pos + 1);
    LOG_DEBUG << "line: " << line;

    // Skip empty lines
    if (line.empty() || line == "\r")
      continue;

    if (line == "data: [DONE]") {
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = false;
      status["is_stream"] = true;
      status["status_code"] = 200;
      (*context->callback)(std::move(status), Json::Value());
      break;
    }

    // Parse the JSON
    Json::Value chunk_json;
    chunk_json["data"] = line + "\n\n";
    Json::Reader reader;

    Json::Value status;
    status["is_done"] = false;
    status["has_error"] = false;
    status["is_stream"] = true;
    status["status_code"] = 200;
    (*context->callback)(std::move(status), std::move(chunk_json));
  }

  return size * nmemb;
}

static size_t WriteCallback(char* ptr, size_t size, size_t nmemb,
                            std::string* data) {
  data->append(ptr, size * nmemb);
  return size * nmemb;
}

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

PythonEngine::PythonEngine() : q_(4 /*n_parallel*/, "python_engine") {}

PythonEngine::~PythonEngine() {
  curl_global_cleanup();
}

void PythonEngine::LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  if (!json_body->isMember("model") || !json_body->isMember("model_dir")) {
    Json::Value error;
    error["error"] = "Missing required fields: model or model_dir";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }

  namespace fs = std::filesystem;

  const std::string& model = (*json_body)["model"].asString();
  const fs::path model_dir = (*json_body)["model_dir"].asString();

  if (models_.find(model) != models_.end()) {
    Json::Value error;
    error["error"] = "Model already loaded!";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k409Conflict;
    callback(std::move(status), std::move(error));
    return;
  }

  pid_t pid;

  try {
    std::vector<std::string> command{GetUvPath(), "run", model_dir / "main.py"};

    // TODO: what happens if the process exits?
    const std::string stdout_path = model_dir / "stdout.txt";
    const std::string stderr_path = model_dir / "stderr.txt";

    // create empty stdout.txt and stderr.txt for redirection
    if (!std::filesystem::exists(stdout_path)) std::ofstream(stdout_path).flush();
    if (!std::filesystem::exists(stderr_path)) std::ofstream(stderr_path).flush();

    pid = cortex::process::SpawnProcess(command, stdout_path, stderr_path);

    process_map_[model] = pid;
    if (pid == -1) {
      throw std::runtime_error("Fail to spawn process with pid -1");
    }
  } catch (const std::exception& e) {
    std::unique_lock lock(models_mutex_);
    if (models_.find(model) != models_.end()) {
      models_.erase(model);
    }

    Json::Value error;
    error["error"] = e.what();
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k500InternalServerError;
    callback(std::move(status), std::move(error));
    return;
  }

  Json::Value response;
  response["status"] =
      "Model loaded successfully with pid: " + std::to_string(pid);
  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;
  callback(std::move(status), std::move(response));
}

void PythonEngine::HandleRequest(
  const std::string& model,
  const std::vector<std::string>& path_parts,
  std::shared_ptr<Json::Value> json_body,
  std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  // get port

  Json::Value response_json;
  response_json["object"] = "list";

  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;

  callback(std::move(status), std::move(response_json));
}

}  // namespace python_engine
