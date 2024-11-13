#include "remote_engine.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
namespace remote_engine {
constexpr const int k200OK = 200;
constexpr const int k400BadRequest = 400;
constexpr const int k409Conflict = 409;
constexpr const int k500InternalServerError = 500;
constexpr const int kFileLoggerOption = 0;

std::string ReplaceApiKeyPlaceholder(const std::string& templateStr,
                                     const std::string& apiKey) {
  const std::string placeholder = "{{api_key}}";
  std::string result = templateStr;
  size_t pos = result.find(placeholder);

  if (pos != std::string::npos) {
    result.replace(pos, placeholder.length(), apiKey);
  }

  return result;
}

static size_t WriteCallback(char* ptr, size_t size, size_t nmemb,
                            std::string* data) {
  data->append(ptr, size * nmemb);
  return size * nmemb;
}

RemoteEngine::RemoteEngine() {
  curl_global_init(CURL_GLOBAL_ALL);
}

RemoteEngine::~RemoteEngine() {
  curl_global_cleanup();
}

RemoteEngine::ModelConfig* RemoteEngine::GetModelConfig(
    const std::string& model) {
  std::shared_lock lock(models_mutex_);
  auto it = models_.find(model);
  if (it != models_.end()) {
    return &it->second;
  }
  return nullptr;
}

CurlResponse RemoteEngine::MakeGetModelsRequest() {
  CURL* curl = curl_easy_init();
  CurlResponse response;

  if (!curl) {
    response.error = true;
    response.error_message = "Failed to initialize CURL";
    return response;
  }

  std::string full_url = metadata_["get_models_url"].asString();

  struct curl_slist* headers = nullptr;

  headers = curl_slist_append(headers, api_key_template_.c_str());
  std::cout << "api_key: " << api_key_template_ << std::endl;

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

CurlResponse RemoteEngine::MakeChatCompletionRequest(
    const ModelConfig& config, const std::string& body,
    const std::string& method) {
  CURL* curl = curl_easy_init();
  CurlResponse response;

  if (!curl) {
    response.error = true;
    response.error_message = "Failed to initialize CURL";
    return response;
  }
  std::string full_url =
      config.transform_req["chat_completions"]["url"].as<std::string>();

  struct curl_slist* headers = nullptr;
  if (!config.api_key.empty()) {

    headers = curl_slist_append(headers, api_key_template_.c_str());
    std::cout << "api_key: " << api_key_template_ << std::endl;
  }
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  if (method == "POST") {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  }

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

bool RemoteEngine::LoadModelConfig(const std::string& model,
                                   const std::string& yaml_path,
                                   const std::string& api_key) {
  try {
    YAML::Node config = YAML::LoadFile(yaml_path);

    ModelConfig model_config;
    model_config.model = model;

    // Required fields
    if (!config["api_key_template"]) {
      LOG_ERROR << "Missing required fields in config for model " << model;
      return false;
    }

    model_config.api_key = api_key;
    // model_config.url = ;
    // Optional fields
    if (config["api_key_template"]) {
      api_key_template_ = ReplaceApiKeyPlaceholder(
          config["api_key_template"].as<std::string>(), api_key);
    }
    if (config["TransformReq"]) {
      model_config.transform_req = config["TransformReq"];
    } else {
      LOG_WARN << "Missing TransformReq in config for model " << model;
    }
    if (config["TransformResp"]) {
      model_config.transform_resp = config["TransformResp"];
    } else {
      LOG_WARN << "Missing TransformResp in config for model " << model;
    }

    model_config.is_loaded = true;

    // Thread-safe update of models map
    {
      std::unique_lock lock(models_mutex_);
      models_[model] = std::move(model_config);
    }

    return true;
  } catch (const YAML::Exception& e) {
    LOG_ERROR << "Failed to load config for model " << model << ": "
              << e.what();
    return false;
  }
}

void RemoteEngine::GetModels(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  auto response = MakeGetModelsRequest();
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
  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;

  callback(std::move(status), std::move(response_json));
}

void RemoteEngine::LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model") || !json_body->isMember("model_path") ||
      !json_body->isMember("api_key")) {
    Json::Value error;
    error["error"] = "Missing required fields: model or model_path";
    callback(Json::Value(), std::move(error));
    return;
  }

  const std::string& model = (*json_body)["model"].asString();
  const std::string& model_path = (*json_body)["model_path"].asString();
  const std::string& api_key = (*json_body)["api_key"].asString();

  if (!LoadModelConfig(model, model_path, api_key)) {
    Json::Value error;
    error["error"] = "Failed to load model configuration";
    callback(Json::Value(), std::move(error));
    return;
  }
  if (json_body->isMember("metadata")) {
    metadata_ = (*json_body)["metadata"];
  }

  Json::Value response;
  response["status"] = "Model loaded successfully";
  callback(Json::Value(), std::move(response));
}

void RemoteEngine::UnloadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model")) {
    Json::Value error;
    error["error"] = "Missing required field: model";
    callback(Json::Value(), std::move(error));
    return;
  }

  const std::string& model = (*json_body)["model"].asString();

  {
    std::unique_lock lock(models_mutex_);
    models_.erase(model);
  }

  Json::Value response;
  response["status"] = "Model unloaded successfully";
  callback(std::move(response), Json::Value());
}

void RemoteEngine::HandleChatCompletion(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model")) {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    Json::Value error;
    error["error"] = "Missing required fields: model";
    callback(std::move(status), std::move(error));
    return;
  }

  const std::string& model = (*json_body)["model"].asString();
  auto* model_config = GetModelConfig(model);

  if (!model_config || !model_config->is_loaded) {
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = k400BadRequest;
    Json::Value error;
    error["error"] = "Model not found or not loaded: " + model;
    callback(std::move(status), std::move(error));
    return;
  }

  Json::FastWriter writer;
  std::string request_body = writer.write((*json_body));
  std::cout << "template: "
            << model_config->transform_req["chat_completions"]["template"]
                   .as<std::string>()
            << std::endl;
  std::string result = renderer_.render(
      model_config->transform_req["chat_completions"]["template"]
          .as<std::string>(),
      (*json_body));

  auto response = MakeChatCompletionRequest(*model_config, result);

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
  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;

  callback(std::move(status), std::move(response_json));
}

void RemoteEngine::GetModelStatus(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model")) {
    Json::Value error;
    error["error"] = "Missing required field: model";
    callback(Json::Value(), std::move(error));
    return;
  }

  const std::string& model = (*json_body)["model"].asString();
  auto* model_config = GetModelConfig(model);

  if (!model_config) {
    Json::Value error;
    error["error"] = "Model not found: " + model;
    callback(Json::Value(), std::move(error));
    return;
  }

  Json::Value response;
  response["model"] = model;
  response["model_loaded"] = model_config->is_loaded;
  response["model_data"] = model_config->url;

  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;
  callback(std::move(status), std::move(response));
}

// Implement remaining virtual functions
void RemoteEngine::HandleEmbedding(
    std::shared_ptr<Json::Value>,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  callback(Json::Value(), Json::Value());
}

bool RemoteEngine::IsSupported(const std::string& f) {
  if (f == "HandleChatCompletion" || f == "LoadModel" || f == "UnloadModel" ||
      f == "GetModelStatus" || f == "GetModels" || f == "SetFileLogger" ||
      f == "SetLogLevel") {
    return true;
  }
  return false;
}

bool RemoteEngine::SetFileLogger(int max_log_lines,
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
}

void RemoteEngine::SetLogLevel(trantor::Logger::LogLevel log_level) {
  trantor::Logger::setLogLevel(log_level);
}

extern "C" {
EngineI* get_engine() {
  return new RemoteEngine();
}
}
}  // namespace remote_engine