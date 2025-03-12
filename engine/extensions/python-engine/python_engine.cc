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

[[maybe_unused]] static size_t WriteCallback(char* ptr, size_t size, size_t nmemb,
                            std::string* data) {
  data->append(ptr, size * nmemb);
  return size * nmemb;
}

}  // namespace

PythonEngine::PythonEngine() : q_(4 /*n_parallel*/, "python_engine") {}

PythonEngine::~PythonEngine() {
  curl_global_cleanup();
}

config::PythonModelConfig* PythonEngine::GetModelConfig(
    const std::string& model) {
  std::shared_lock lock(models_mutex_);
  auto it = models_.find(model);
  if (it != models_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool PythonEngine::TerminateModelProcess(const std::string& model) {
  auto it = process_map_.find(model);
  if (it == process_map_.end()) {
    LOG_ERROR << "No process found for model: " << model
              << ", removing from list running models.";
    models_.erase(model);
    return false;
  }

  bool success = cortex::process::KillProcess(it->second);
  if (success) {
    process_map_.erase(it);
  }
  return success;
}

CurlResponse PythonEngine::MakeGetRequest(const std::string& model,
                                          const std::string& path) {
  auto const& config = models_[model];
  std::string full_url = "http://localhost:" + config.port + path;
  CurlResponse response;

  auto result = curl_utils::SimpleRequest(full_url, RequestType::GET);
  if (result.has_error()) {
    response.error = true;
    response.error_message = result.error();
  } else {
    response.body = result.value();
  }
  return response;
}

CurlResponse PythonEngine::MakeDeleteRequest(const std::string& model,
                                             const std::string& path) {
  auto const& config = models_[model];
  std::string full_url = "http://localhost:" + config.port + path;
  CurlResponse response;

  auto result = curl_utils::SimpleRequest(full_url, RequestType::DEL);

  if (result.has_error()) {
    response.error = true;
    response.error_message = result.error();
  } else {
    response.body = result.value();
  }

  return response;
}

CurlResponse PythonEngine::MakePostRequest(const std::string& model,
                                           const std::string& path,
                                           const std::string& body) {
  auto const& config = models_[model];
  std::string full_url = "http://localhost:" + config.port + path;

  CurlResponse response;
  auto result = curl_utils::SimpleRequest(full_url, RequestType::POST, body);

  if (result.has_error()) {
    response.error = true;
    response.error_message = result.error();
  } else {
    response.body = result.value();
  }
  return response;
}

bool PythonEngine::LoadModelConfig(const std::string& model,
                                   const std::string& yaml_path) {
  try {
    config::PythonModelConfig config;
    config.ReadFromYaml(yaml_path);
    std::unique_lock lock(models_mutex_);
    models_[model] = config;
  } catch (const std::exception& e) {
    LOG_ERROR << "Failed to load model config: " << e.what();
    return false;
  }

  return true;
}

void PythonEngine::GetModels(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  Json::Value response_json;
  Json::Value model_array(Json::arrayValue);

  for (const auto& pair : models_) {
    auto val = pair.second.ToJson();
    model_array.append(val);
  }

  response_json["object"] = "list";
  response_json["data"] = model_array;

  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;

  callback(std::move(status), std::move(response_json));
  (void) json_body;
}

void PythonEngine::LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  // TODO: handle a case that can spawn process but the process spawn fail.
  pid_t pid;
  if (!json_body->isMember("model") || !json_body->isMember("model_path")) {
    Json::Value error;
    error["error"] = "Missing required fields: model or model_path";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }

  const std::string& model = (*json_body)["model"].asString();
  const std::string& model_path = (*json_body)["model_path"].asString();
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

  if (!LoadModelConfig(model, model_path)) {
    Json::Value error;
    error["error"] = "Failed to load model configuration";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k500InternalServerError;
    callback(std::move(status), std::move(error));
    return;
  }
  auto model_config = models_[model];
  auto model_folder_path = model_config.files[0];
  auto data_folder_path =
      std::filesystem::path(model_folder_path) / std::filesystem::path("venv");
  try {
#if defined(_WIN32)
    auto executable = std::filesystem::path(data_folder_path) /
                      std::filesystem::path("Scripts");
#else
    auto executable =
        std::filesystem::path(data_folder_path) / std::filesystem::path("bin");
#endif

    auto executable_str =
        (executable / std::filesystem::path(model_config.command[0])).string();
    auto command = model_config.command;
    command[0] = executable_str;
    command.push_back((std::filesystem::path(model_folder_path) /
                       std::filesystem::path(model_config.script))
                          .string());
    std::list<std::string> args{"--port",
                                model_config.port,
                                "--log_path",
                                (file_manager_utils::GetCortexLogPath() /
                                 std::filesystem::path(model_config.log_path))
                                    .string(),
                                "--log_level",
                                model_config.log_level};
    if (!model_config.extra_params.isNull() &&
        model_config.extra_params.isObject()) {
      for (const auto& key : model_config.extra_params.getMemberNames()) {
        const Json::Value& value = model_config.extra_params[key];

        // Convert key to string with -- prefix
        std::string param_key = "--" + key;

        // Handle different JSON value types
        if (value.isString()) {
          args.emplace_back(param_key);
          args.emplace_back(value.asString());
        } else if (value.isInt()) {
          args.emplace_back(param_key);
          args.emplace_back(std::to_string(value.asInt()));
        } else if (value.isDouble()) {
          args.emplace_back(param_key);
          args.emplace_back(std::to_string(value.asDouble()));
        } else if (value.isBool()) {
          // For boolean, only add the flag if true
          if (value.asBool()) {
            args.emplace_back(param_key);
          }
        }
      }
    }

    // Add the parsed arguments to the command
    command.insert(command.end(), args.begin(), args.end());
    auto result = cortex::process::SpawnProcess(command);

    if (result.has_error()) {
      CTL_ERR("Fail to spawn process. " << result.error());

      std::unique_lock lock(models_mutex_);
      if (models_.find(model) != models_.end()) {
        models_.erase(model);
      }

      Json::Value error;
      error["error"] = "Fail to spawn process";
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = true;
      status["is_stream"] = false;
      status["status_code"] = k500InternalServerError;
      callback(std::move(status), std::move(error));
      return;
    }

    pid = result.value().pid;
    process_map_[model] = result.value();
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

void PythonEngine::UnloadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model")) {
    Json::Value error;
    error["error"] = "Missing required field: model";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }

  auto model = (*json_body)["model"].asString();

  {
    if (TerminateModelProcess(model)) {
      std::unique_lock lock(models_mutex_);
      models_.erase(model);
    } else {
      Json::Value error;
      error["error"] = "Fail to terminate process with id: " +
                       std::to_string(process_map_[model].pid);
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = true;
      status["is_stream"] = false;
      status["status_code"] = k400BadRequest;
      callback(std::move(status), std::move(error));
      return;
    }
  }

  Json::Value response;
  response["status"] = "Model unloaded successfully";
  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;
  callback(std::move(status), std::move(response));
}

void PythonEngine::HandleChatCompletion(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  LOG_WARN << "Does not support yet!";
  (void) json_body;
  (void) callback;
}

CurlResponse PythonEngine::MakeStreamPostRequest(
    const std::string& model, const std::string& path, const std::string& body,
    const std::function<void(Json::Value&&, Json::Value&&)>& callback) {
  auto const& config = models_[model];
  CURL* curl = curl_easy_init();
  CurlResponse response;

  if (!curl) {
    response.error = true;
    response.error_message = "Failed to initialize CURL";
    return response;
  }

  std::string full_url = "http://localhost:" + config.port + path;

  struct curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "Accept: text/event-stream");
  headers = curl_slist_append(headers, "Cache-Control: no-cache");
  headers = curl_slist_append(headers, "Connection: keep-alive");

  StreamContext context{
      std::make_shared<std::function<void(Json::Value&&, Json::Value&&)>>(
          callback),
      ""};

  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &context);
  curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 1L);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    response.error = true;
    response.error_message = curl_easy_strerror(res);

    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = true;
    status["status_code"] = 500;

    Json::Value error;
    error["error"] = response.error_message;
    callback(std::move(status), std::move(error));
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return response;
}

void PythonEngine::HandleInference(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (json_body && !json_body->isMember("model")) {
    Json::Value error;
    error["error"] = "Missing required field: model is required!";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }

  std::string method = "post";
  std::string path = "/inference";
  auto transform_request = (*json_body).get("transform_request", "").asString();
  auto transform_response =
      (*json_body).get("transform_response", "").asString();
  auto model = (*json_body)["model"].asString();
  auto& body = (*json_body)["body"];

  if (models_.find(model) == models_.end()) {
    Json::Value error;
    error["error"] = "Model '" + model + "' is not loaded!";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }

  // Transform Request
  std::string transformed_request;
  if (!transform_request.empty()) {

    try {
      // Validate JSON body
      if (!body || body.isNull()) {
        throw std::runtime_error("Invalid or null JSON body");
      }

      // Render with error handling
      try {
        transformed_request = renderer_.Render(transform_request, body);

      } catch (const std::exception& e) {
        throw std::runtime_error("Template rendering error: " +
                                 std::string(e.what()));
      }
    } catch (const std::exception& e) {
      // Log error and potentially rethrow or handle accordingly
      LOG_WARN << "Error in TransformRequest: " << e.what();
      LOG_WARN << "Using original request body";
      transformed_request = body.toStyledString();
    }
  } else {
    transformed_request = body.toStyledString();
  }

  // End Transform request

  CurlResponse response;
  if (method == "post") {
    if (body.isMember("stream") && body["stream"].asBool()) {
      q_.runTaskInQueue(
          [this, model, path, transformed_request, cb = std::move(callback)] {
            MakeStreamPostRequest(model, path, transformed_request, cb);
          });

      return;
    } else {
      response = MakePostRequest(model, path, transformed_request);
    }

  } else if (method == "get") {
    response = MakeGetRequest(model, path);
  } else if (method == "delete") {
    response = MakeDeleteRequest(model, path);
  } else {
    Json::Value error;
    error["error"] =
        "method not supported! Supported methods are: post, get, delete";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }

  if (response.error) {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    Json::Value error;
    error["error"] = response.error_message;
    callback(std::move(status), std::move(error));
    return;
  }

  Json::Value response_json;
  Json::Reader reader;
  if (!reader.parse(response.body, response_json)) {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k500InternalServerError;
    Json::Value error;
    error["error"] = "Failed to parse response";
    callback(std::move(status), std::move(error));
    return;
  }

  if (!transform_response.empty()) {
    // Transform Response
    std::string response_str;
    try {
      // Validate JSON body
      if (!response_json || response_json.isNull()) {
        throw std::runtime_error("Invalid or null JSON body");
      }
      // Render with error handling
      try {
        response_str = renderer_.Render(transform_response, response_json);
      } catch (const std::exception& e) {
        throw std::runtime_error("Template rendering error: " +
                                 std::string(e.what()));
      }
    } catch (const std::exception& e) {
      // Log error and potentially rethrow or handle accordingly
      LOG_WARN << "Error in TransformRequest: " << e.what();
      LOG_WARN << "Using original request body";
      response_str = response_json.toStyledString();
    }

    Json::Reader reader_final;
    Json::Value response_json_final;
    if (!reader_final.parse(response_str, response_json_final)) {
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = true;
      status["is_stream"] = false;
      status["status_code"] = k500InternalServerError;
      Json::Value error;
      error["error"] = "Failed to parse response";
      callback(std::move(status), std::move(error));
      return;
    }

    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = false;
    status["is_stream"] = false;
    status["status_code"] = k200OK;

    callback(std::move(status), std::move(response_json_final));
  } else {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = false;
    status["is_stream"] = false;
    status["status_code"] = k200OK;

    callback(std::move(status), std::move(response_json));
  }
}

Json::Value PythonEngine::GetRemoteModels() {
  return Json::Value();
}

void PythonEngine::StopInferencing(const std::string& model_id) {
  (void)model_id;
}

void PythonEngine::HandleRouteRequest(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model") || !json_body->isMember("method") ||
      !json_body->isMember("path")) {
    Json::Value error;
    error["error"] =
        "Missing required field: model, method and path are required!";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }
  auto method = (*json_body)["method"].asString();
  auto path = (*json_body)["path"].asString();
  auto transform_request = (*json_body).get("transform_request", "").asString();
  auto transform_response =
      (*json_body).get("transform_response", "").asString();
  auto model = (*json_body)["model"].asString();
  auto& body = (*json_body)["body"];

  if (models_.find(model) == models_.end()) {
    Json::Value error;
    error["error"] = "Model '" + model + "' is not loaded!";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }

  // Transform Request
  std::string transformed_request;
  if (!transform_request.empty()) {

    try {
      // Validate JSON body
      if (!body || body.isNull()) {
        throw std::runtime_error("Invalid or null JSON body");
      }

      // Render with error handling
      try {
        transformed_request = renderer_.Render(transform_request, *json_body);
      } catch (const std::exception& e) {
        throw std::runtime_error("Template rendering error: " +
                                 std::string(e.what()));
      }
    } catch (const std::exception& e) {
      // Log error and potentially rethrow or handle accordingly
      LOG_WARN << "Error in TransformRequest: " << e.what();
      LOG_WARN << "Using original request body";
      transformed_request = body.toStyledString();
    }
  } else {
    transformed_request = body.toStyledString();
  }

  // End Transform request

  CurlResponse response;
  if (method == "post") {
    response = MakePostRequest(model, path, transformed_request);
  } else if (method == "get") {
    response = MakeGetRequest(model, path);
  } else if (method == "delete") {
    response = MakeDeleteRequest(model, path);
  } else {
    Json::Value error;
    error["error"] =
        "method not supported! Supported methods are: post, get, delete";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }

  if (response.error) {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    Json::Value error;
    error["error"] = response.error_message;
    callback(std::move(status), std::move(error));
    return;
  }

  Json::Value response_json;
  Json::Reader reader;
  if (!reader.parse(response.body, response_json)) {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k500InternalServerError;
    Json::Value error;
    error["error"] = "Failed to parse response";
    callback(std::move(status), std::move(error));
    return;
  }

  if (!transform_response.empty()) {
    // Transform Response
    std::string response_str;
    try {
      // Validate JSON body
      if (!response_json || response_json.isNull()) {
        throw std::runtime_error("Invalid or null JSON body");
      }
      // Render with error handling
      try {
        response_str = renderer_.Render(transform_response, response_json);
      } catch (const std::exception& e) {
        throw std::runtime_error("Template rendering error: " +
                                 std::string(e.what()));
      }
    } catch (const std::exception& e) {
      // Log error and potentially rethrow or handle accordingly
      LOG_WARN << "Error in TransformRequest: " << e.what();
      LOG_WARN << "Using original request body";
      response_str = response_json.toStyledString();
    }

    Json::Reader reader_final;
    Json::Value response_json_final;
    if (!reader_final.parse(response_str, response_json_final)) {
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = true;
      status["is_stream"] = false;
      status["status_code"] = k500InternalServerError;
      Json::Value error;
      error["error"] = "Failed to parse response";
      callback(std::move(status), std::move(error));
      return;
    }

    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = false;
    status["is_stream"] = false;
    status["status_code"] = k200OK;

    callback(std::move(status), std::move(response_json_final));
  } else {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = false;
    status["is_stream"] = false;
    status["status_code"] = k200OK;

    callback(std::move(status), std::move(response_json));
  }
}

void PythonEngine::GetModelStatus(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model")) {
    Json::Value error;
    error["error"] = "Missing required field: model";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    callback(std::move(status), std::move(error));
    return;
  }

  auto model = json_body->get("model", "").asString();
  auto model_config = models_[model];
  auto health_endpoint = model_config.heath_check;
  auto pid = process_map_[model];
  auto is_process_live = cortex::process::IsProcessAlive(pid);
  auto response_health = MakeGetRequest(model, health_endpoint.path);

  if (response_health.error && is_process_live) {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = false;
    status["is_stream"] = false;
    status["status_code"] = k200OK;
    Json::Value message;
    message["message"] = "model '"+model+"' is loading";
    callback(std::move(status), std::move(message));
    return;
  }
  else if(response_health.error && !is_process_live){
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    Json::Value message;
    message["message"] = response_health.error_message;
    callback(std::move(status), std::move(message));
    return;
  }

  Json::Value response;
  response["model"] = model;
  response["model_loaded"] = true;
  response["model_data"] = model_config.ToJson();

  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;
  callback(std::move(status), std::move(response));
}

// Implement remaining virtual functions
void PythonEngine::HandleEmbedding(
    std::shared_ptr<Json::Value>,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  callback(Json::Value(), Json::Value());
}

bool PythonEngine::IsSupported(const std::string& f) {
  if (f == "HandleChatCompletion" || f == "LoadModel" || f == "UnloadModel" ||
      f == "GetModelStatus" || f == "GetModels" || f == "SetFileLogger" ||
      f == "SetLogLevel") {
    return true;
  }
  return false;
}

bool PythonEngine::SetFileLogger(int max_log_lines,
                                 const std::string& log_path) {
  if (!async_file_logger_) {
    async_file_logger_ = std::make_unique<trantor::FileLogger>();
  }

  async_file_logger_->setFileName(log_path);
  async_file_logger_->setMaxLines(max_log_lines);  // Keep last 100000 lines
  async_file_logger_->startLogging();
  trantor::Logger::setOutputFunction(
      [&](const char* msg, const uint64_t len) {
        if (async_file_logger_)
          async_file_logger_->output_(msg, len);
      },
      [&]() {
        if (async_file_logger_)
          async_file_logger_->flush();
      });
  freopen(log_path.c_str(), "w", stderr);
  freopen(log_path.c_str(), "w", stdout);
  return true;
}

void PythonEngine::SetLogLevel(trantor::Logger::LogLevel log_level) {
  trantor::Logger::setLogLevel(log_level);
}

void PythonEngine::Load(EngineLoadOption opts) {
  // Develop register model here on loading engine
  (void) opts;
};

void PythonEngine::Unload(EngineUnloadOption opts) {
  for (const auto& pair : models_) {
    TerminateModelProcess(pair.first);
  }
  (void) opts;
};

}  // namespace python_engine
