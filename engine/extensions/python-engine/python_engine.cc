#include "python_engine.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
namespace python_engine {
constexpr const int k200OK = 200;
constexpr const int k400BadRequest = 400;
constexpr const int k409Conflict = 409;
constexpr const int k500InternalServerError = 500;
constexpr const int kFileLoggerOption = 0;

static size_t WriteCallback(char* ptr, size_t size, size_t nmemb,
                            std::string* data) {
  data->append(ptr, size * nmemb);
  return size * nmemb;
}

PythonEngine::PythonEngine():q_(4 /*n_parallel*/, "python_engine") {
  curl_global_init(CURL_GLOBAL_ALL);
}

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
std::string constructWindowsCommandLine(const std::vector<std::string>& args) {
  std::string cmdLine;
  for (const auto& arg : args) {
    // Simple escaping for Windows command line
    std::string escapedArg = arg;
    if (escapedArg.find(' ') != std::string::npos) {
      // Wrap in quotes and escape existing quotes
      for (char& c : escapedArg) {
        if (c == '"')
          c = '\\';
      }
      escapedArg = "\"" + escapedArg + "\"";
    }
    cmdLine += escapedArg + " ";
  }
  return cmdLine;
}

std::vector<char*> convertToArgv(const std::vector<std::string>& args) {
  std::vector<char*> argv;
  for (const auto& arg : args) {
    argv.push_back(const_cast<char*>(arg.c_str()));
  }
  argv.push_back(nullptr);
  return argv;
}

pid_t PythonEngine::SpawnProcess(const std::string& model,
                                 const std::vector<std::string>& command) {
  try {
#ifdef _WIN32
    // Windows process creation
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    // Construct command line
    std::string cmdLine = constructWindowsCommandLine(command);

    // Convert string to char* for Windows API
    char commandBuffer[4096];
    strncpy_s(commandBuffer, cmdLine.c_str(), sizeof(commandBuffer));

    if (!CreateProcessA(NULL,           // lpApplicationName
                        commandBuffer,  // lpCommandLine
                        NULL,           // lpProcessAttributes
                        NULL,           // lpThreadAttributes
                        FALSE,          // bInheritHandles
                        0,              // dwCreationFlags
                        NULL,           // lpEnvironment
                        NULL,           // lpCurrentDirectory
                        &si,            // lpStartupInfo
                        &pi             // lpProcessInformation
                        )) {
      throw std::runtime_error("Failed to create process on Windows");
    }

    // Store the process ID
    pid_t pid = pi.dwProcessId;
    processMap[model] = pid;

    // Close handles to avoid resource leaks
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return pid;

#elif __APPLE__ || __linux__
    // POSIX process creation
    pid_t pid;

    // Convert command vector to char*[]
    std::vector<char*> argv = convertToArgv(command);
    // for (auto c : command) {
    //   std::cout << c << " " << std::endl;
    // }

    // Use posix_spawn for cross-platform compatibility
    int spawn_result = posix_spawn(&pid,                // pid output
                                   command[0].c_str(),  // executable path
                                   NULL,                // file actions
                                   NULL,                // spawn attributes
                                   argv.data(),         // argument vector
                                   NULL                 // environment (inherit)
    );

    if (spawn_result != 0) {
      throw std::runtime_error("Failed to spawn process");
    }

    // Store the process ID
    processMap[model] = pid;
    return pid;

#else
#error Unsupported platform
#endif
  } catch (const std::exception& e) {
    LOG_ERROR << "Process spawning error: " << e.what();
    return -1;
  }
}
bool PythonEngine::TerminateModelProcess(const std::string& model) {
  auto it = processMap.find(model);
  if (it == processMap.end()) {
    LOG_ERROR << "No process found for model: " << model
              << ", removing from list running models.";
    models_.erase(model);
    return false;
  }

#ifdef _WIN32
  HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, it->second);
  if (hProcess == NULL) {
    LOG_ERROR << "Failed to open process";
    return false;
  }

  bool terminated = TerminateProcess(hProcess, 0) == TRUE;
  CloseHandle(hProcess);

  if (terminated) {
    processMap.erase(it);
    return true;
  }

#elif __APPLE__ || __linux__
  int result = kill(it->second, SIGTERM);
  if (result == 0) {
    processMap.erase(it);
    return true;
  }
#endif

  return false;
}
CurlResponse PythonEngine::MakeGetRequest(const std::string& model,
                                          const std::string& path) {
  auto config = models_[model];
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

  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  std::string response_string;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    response.error = true;
    response.error_message = curl_easy_strerror(res);
  } else {
    response.body = response_string;
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return response;
}
CurlResponse PythonEngine::MakeDeleteRequest(const std::string& model,
                                             const std::string& path) {
  auto config = models_[model];
  CURL* curl = curl_easy_init();
  CurlResponse response;

  if (!curl) {
    response.error = true;
    response.error_message = "Failed to initialize CURL";
    return response;
  }
  std::string full_url = "http://localhost:" + config.port + path;

  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

  std::string response_string;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    response.error = true;
    response.error_message = curl_easy_strerror(res);
  } else {
    response.body = response_string;
  }

  curl_easy_cleanup(curl);
  return response;
}

CurlResponse PythonEngine::MakePostRequest(const std::string& model,
                                           const std::string& path,
                                           const std::string& body) {
  auto config = models_[model];
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

  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

  std::string response_string;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    response.error = true;
    response.error_message = curl_easy_strerror(res);
  } else {
    response.body = response_string;
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
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
#ifdef _WIN32
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
    pid = SpawnProcess(model, command);
    if (pid == -1) {
      std::unique_lock lock(models_mutex_);
      if (models_.find(model) != models_.end()) {
        models_.erase(model);
      }

      Json::Value error;
      error["error"] = "Fail to spawn process with pid -1";
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = true;
      status["is_stream"] = false;
      status["status_code"] = k500InternalServerError;
      callback(std::move(status), std::move(error));
      return;
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

  const std::string& model = (*json_body)["model"].asString();

  {
    std::unique_lock lock(models_mutex_);
    if (TerminateModelProcess(model)) {
      models_.erase(model);
    } else {
      Json::Value error;
      error["error"] = "Fail to terminate process with id: " +
                       std::to_string(processMap[model]);
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
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {}

CurlResponse PythonEngine::MakeStreamPostRequest(
    const std::string& model, const std::string& path, const std::string& body,
    const std::function<void(Json::Value&&, Json::Value&&)>& callback) {
  auto config = models_[model];
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
  if (!json_body->isMember("model")) {
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
  std::string transform_request =
      (*json_body).get("transform_request", "").asString();
  std::string transform_response =
      (*json_body).get("transform_response", "").asString();
  std::string model = (*json_body)["model"].asString();
  Json::Value body = (*json_body)["body"];

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
void PythonEngine::StopInferencing(const std::string& model_id) {}
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
  std::string method = (*json_body)["method"].asString();
  std::string path = (*json_body)["path"].asString();
  std::string transform_request =
      (*json_body).get("transform_request", "").asString();
  std::string transform_response =
      (*json_body).get("transform_response", "").asString();
  std::string model = (*json_body)["model"].asString();
  Json::Value body = (*json_body)["body"];

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
  auto response_health = MakeGetRequest(model, health_endpoint.path);

  if (response_health.error) {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    Json::Value error;
    error["error"] = response_health.error_message;
    callback(std::move(status), std::move(error));
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
};

void PythonEngine::Unload(EngineUnloadOption opts) {};

// extern "C" {
// EngineI* get_engine() {
//   return new PythonEngine();
// }
// }
}  // namespace python_engine