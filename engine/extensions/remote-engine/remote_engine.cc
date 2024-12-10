#include "remote_engine.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
namespace remote_engine {
namespace {
constexpr const int k200OK = 200;
constexpr const int k400BadRequest = 400;
constexpr const int k409Conflict = 409;
constexpr const int k500InternalServerError = 500;
constexpr const int kFileLoggerOption = 0;
bool is_anthropic(const std::string& model) {
  return model.find("claude") != std::string::npos;
}

bool is_openai(const std::string& model) {
  return model.find("gpt") != std::string::npos;
}

}  // namespace

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
    // CTL_INF(line);

    // Skip empty lines
    if (line.empty() || line == "\r" ||
        line.find("event:") != std::string::npos)
      continue;

    // Skip [DONE] message
    // std::cout << line << std::endl;
    CTL_DBG(line);
    if (line == "data: [DONE]" ||
        line.find("message_stop") != std::string::npos) {
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
    if (!is_openai(context->model)) {
      std::string s = line.substr(6);
      try {
        auto root = json_helper::ParseJsonString(s);
        root["model"] = context->model;
        root["id"] = context->id;
        root["stream"] = true;
        auto result = context->renderer.Render(context->stream_template, root);
        CTL_DBG(result);
        chunk_json["data"] = "data: " + result + "\n\n";
      } catch (const std::exception& e) {
        CTL_WRN("JSON parse error: " << e.what());
        continue;
      }
    } else {
      chunk_json["data"] = line + "\n\n";
    }
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

CurlResponse RemoteEngine::MakeStreamingChatCompletionRequest(
    const ModelConfig& config, const std::string& body,
    const std::function<void(Json::Value&&, Json::Value&&)>& callback) {

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
  }

  if (is_anthropic(config.model)) {
    std::string v = "anthropic-version: " + config.version;
    headers = curl_slist_append(headers, v.c_str());
  }

  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "Accept: text/event-stream");
  headers = curl_slist_append(headers, "Cache-Control: no-cache");
  headers = curl_slist_append(headers, "Connection: keep-alive");

  std::string stream_template = chat_res_template_;

  StreamContext context{
      std::make_shared<std::function<void(Json::Value&&, Json::Value&&)>>(
          callback),
      "",
      "",
      config.model,
      renderer_,
      stream_template};

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
  std::shared_lock lock(models_mtx_);
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
  }

  if (is_anthropic(config.model)) {
    std::string v = "anthropic-version: " + config.version;
    headers = curl_slist_append(headers, v.c_str());
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
    if (is_anthropic(model)) {
      if (!config["version"]) {
        CTL_ERR("Missing version for model: " << model);
        return false;
      }
      model_config.version = config["version"].as<std::string>();
    }

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
      std::unique_lock lock(models_mtx_);
      models_[model] = std::move(model_config);
    }
    CTL_DBG("LoadModelConfig successfully: " << model << ", " << yaml_path);

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
  CTL_WRN("Not implemented yet!");
}

void RemoteEngine::LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model") || !json_body->isMember("model_path") ||
      !json_body->isMember("api_key")) {
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
  const std::string& api_key = (*json_body)["api_key"].asString();

  if (!LoadModelConfig(model, model_path, api_key)) {
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
  if (json_body->isMember("metadata")) {
    metadata_ = (*json_body)["metadata"];
    if (!metadata_["TransformReq"].isNull() &&
        !metadata_["TransformReq"]["chat_completions"].isNull() &&
        !metadata_["TransformReq"]["chat_completions"]["template"].isNull()) {
      chat_req_template_ =
          metadata_["TransformReq"]["chat_completions"]["template"].asString();
      CTL_INF(chat_req_template_);
    }

    if (!metadata_["TransformResp"].isNull() &&
        !metadata_["TransformResp"]["chat_completions"].isNull() &&
        !metadata_["TransformResp"]["chat_completions"]["template"].isNull()) {
      chat_res_template_ =
          metadata_["TransformResp"]["chat_completions"]["template"].asString();
      CTL_INF(chat_res_template_);
    }
  }

  Json::Value response;
  response["status"] = "Model loaded successfully";
  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = k200OK;
  callback(std::move(status), std::move(response));
  CTL_INF("Model loaded successfully: " << model);
}

void RemoteEngine::UnloadModel(
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
    std::unique_lock lock(models_mtx_);
    models_.erase(model);
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
  bool is_stream =
      json_body->isMember("stream") && (*json_body)["stream"].asBool();
  Json::FastWriter writer;
  // Transform request
  std::string result;
  try {
    // Check if required YAML nodes exist
    if (!model_config->transform_req["chat_completions"]) {
      throw std::runtime_error(
          "Missing 'chat_completions' node in transform_req");
    }
    if (!model_config->transform_req["chat_completions"]["template"]) {
      throw std::runtime_error("Missing 'template' node in chat_completions");
    }

    // Validate JSON body
    if (!json_body || json_body->isNull()) {
      throw std::runtime_error("Invalid or null JSON body");
    }

    // Get template string with error check
    std::string template_str;
    try {
      template_str = model_config->transform_req["chat_completions"]["template"]
                         .as<std::string>();
    } catch (const YAML::BadConversion& e) {
      throw std::runtime_error("Failed to convert template node to string: " +
                               std::string(e.what()));
    }

    // Parse system for anthropic
    if (is_anthropic(model)) {
      bool has_system = false;
      Json::Value msgs(Json::arrayValue);
      for (auto& kv : (*json_body)["messages"]) {
        if (kv["role"].asString() == "system") {
          (*json_body)["system"] = kv["content"].asString();
          has_system = true;
        } else {
          msgs.append(kv);
        }
      }
      if (has_system) {
        (*json_body)["messages"] = msgs;
      }
    }

    // Render with error handling
    try {
      result = renderer_.Render(template_str, *json_body);
    } catch (const std::exception& e) {
      throw std::runtime_error("Template rendering error: " +
                               std::string(e.what()));
    }
  } catch (const std::exception& e) {
    // Log error and potentially rethrow or handle accordingly
    LOG_WARN << "Error in TransformRequest: " << e.what();
    LOG_WARN << "Using original request body";
    result = (*json_body).toStyledString();
  }

  if (is_stream) {
    MakeStreamingChatCompletionRequest(*model_config, result, callback);
  } else {

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

    // Transform Response
    std::string response_str;
    try {
      std::string template_str;
      if (!chat_res_template_.empty()) {
        CTL_DBG(
            "Use engine transform response template: " << chat_res_template_);
        template_str = chat_res_template_;
      } else {
        // Check if required YAML nodes exist
        if (!model_config->transform_resp["chat_completions"]) {
          throw std::runtime_error(
              "Missing 'chat_completions' node in transform_resp");
        }
        if (!model_config->transform_resp["chat_completions"]["template"]) {
          throw std::runtime_error(
              "Missing 'template' node in chat_completions");
        }

        // Validate JSON body
        if (!response_json || response_json.isNull()) {
          throw std::runtime_error("Invalid or null JSON body");
        }

        // Get template string with error check

        try {
          template_str =
              model_config->transform_resp["chat_completions"]["template"]
                  .as<std::string>();
        } catch (const YAML::BadConversion& e) {
          throw std::runtime_error(
              "Failed to convert template node to string: " +
              std::string(e.what()));
        }
      }

      try {
        response_json["stream"] = false;
        response_str = renderer_.Render(template_str, response_json);
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
  }
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

Json::Value RemoteEngine::GetRemoteModels() {
  CTL_WRN("Not implemented yet!");
  return {};
}

}  // namespace remote_engine