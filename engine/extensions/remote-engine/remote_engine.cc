#include "remote_engine.h"
#include <filesystem>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include "helper.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
namespace remote_engine {
namespace {
constexpr const int k200OK = 200;
constexpr const int k400BadRequest = 400;
constexpr const int k409Conflict = 409;
constexpr const int k500InternalServerError = 500;
constexpr const int kFileLoggerOption = 0;

constexpr const std::array<std::string_view, 5> kAnthropicModels = {
    "claude-3-5-sonnet-20241022", "claude-3-5-haiku-20241022",
    "claude-3-opus-20240229", "claude-3-sonnet-20240229",
    "claude-3-haiku-20240307"};

}  // namespace

size_t StreamWriteCallback(char* ptr, size_t size, size_t nmemb,
                           void* userdata) {
  auto* context = static_cast<StreamContext*>(userdata);
  std::string chunk(ptr, size * nmemb);
  CTL_DBG(chunk);
  Json::Value check_error;
  Json::Reader reader;
  context->chunks += chunk;

  long http_code = k200OK;
  if (context->curl) {
    curl_easy_getinfo(context->curl, CURLINFO_RESPONSE_CODE, &http_code);
  }
  if (http_code != k200OK && (reader.parse(context->chunks, check_error) ||
                              (chunk.find("error") != std::string::npos &&
                               reader.parse(chunk, check_error)))) {
    CTL_WRN(context->chunks);
    CTL_WRN("http code: " << http_code << " - " << chunk);
    CTL_INF("Request: " << context->last_request);
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = true;
    status["status_code"] = k400BadRequest;
    context->need_stop = false;
    (*context->callback)(std::move(status), std::move(check_error));
    return size * nmemb;
  }

  context->buffer += chunk;
  // Process complete lines
  size_t pos;
  while ((pos = context->buffer.find('\n')) != std::string::npos) {
    std::string line = context->buffer.substr(0, pos);
    context->buffer = context->buffer.substr(pos + 1);

    // Skip empty lines
    if (line.empty() || line == "\r" ||
        line.find("event:") != std::string::npos)
      continue;

    CTL_DBG(line);
    if (line == "data: [DONE]" ||
        line.find("message_stop") != std::string::npos) {
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = false;
      status["is_stream"] = true;
      status["status_code"] = k200OK;
      context->need_stop = false;
      (*context->callback)(std::move(status), Json::Value());
      break;
    }

    // Parse the JSON
    Json::Value chunk_json;
    std::string s = line;
    if (line.size() > 6)
      s = line.substr(6);
    try {
      auto root = json_helper::ParseJsonString(s);
      if (root.getMemberNames().empty())
        continue;
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

  std::string full_url = chat_url_;
  CTL_DBG("full_url: " << full_url);

  struct curl_slist* headers = nullptr;
  for (auto const& h : header_) {
    headers = curl_slist_append(headers, h.c_str());
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
      stream_template,
      true,
      body,
      "",
      curl};

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
  if (context.need_stop) {
    CTL_DBG("No stop message received, need to stop");
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = false;
    status["is_stream"] = true;
    status["status_code"] = k200OK;
    (*context.callback)(std::move(status), Json::Value());
  }
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

std::vector<std::string> ReplaceHeaderPlaceholders(
    const std::string& template_str, Json::Value json_body) {
  CTL_DBG(template_str);
  auto keys = GetReplacements(template_str);
  if (keys.empty())
    return std::vector<std::string>{};
  std::unordered_map<std::string, std::string> replacements;
  for (auto const& k : keys) {
    if (json_body.isMember(k)) {
      replacements.insert({k, json_body[k].asString()});
    }
  }
  return ReplaceHeaderPlaceholders(template_str, replacements);
}

static size_t WriteCallback(char* ptr, size_t size, size_t nmemb,
                            std::string* data) {
  data->append(ptr, size * nmemb);
  return size * nmemb;
}

RemoteEngine::RemoteEngine(const std::string& engine_name)
    : engine_name_(engine_name), q_(1 /*n_parallel*/, engine_name) {
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

CurlResponse RemoteEngine::MakeGetModelsRequest(
    const std::string& url, const std::string& api_key,
    const std::string& header_template) {
  CURL* curl = curl_easy_init();
  CurlResponse response;

  if (!curl) {
    response.error = true;
    response.error_message = "Failed to initialize CURL";
    return response;
  }

  std::unordered_map<std::string, std::string> replacements = {
      {"api_key", api_key}};
  auto hs = ReplaceHeaderPlaceholders(header_template, replacements);

  struct curl_slist* headers = nullptr;
  for (auto const& h : hs) {
    headers = curl_slist_append(headers, h.c_str());
  }
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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
  std::string full_url = chat_url_;
  CTL_DBG("full_url: " << full_url);

  struct curl_slist* headers = nullptr;
  for (auto const& h : header_) {
    headers = curl_slist_append(headers, h.c_str());
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
                                   const Json::Value& body) {
  try {
    YAML::Node config = YAML::LoadFile(yaml_path);

    ModelConfig model_config;
    model_config.model = model;
    // model_config.url = ;
    // Optional fields
    if (auto s = config["header_template"]; s && !s.as<std::string>().empty()) {
      header_ = ReplaceHeaderPlaceholders(s.as<std::string>(), body);
      for (auto const& h : header_) {
        CTL_DBG("header: " << h);
      }
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
  Json::Value json_resp;
  Json::Value model_array(Json::arrayValue);
  {
    std::shared_lock l(models_mtx_);
    for (const auto& [m, _] : models_) {
      Json::Value val;
      val["id"] = m;
      val["engine"] = "openai";
      val["start_time"] = "_";
      val["model_size"] = "_";
      val["vram"] = "_";
      val["ram"] = "_";
      val["object"] = "model";
      model_array.append(val);
    }
  }

  json_resp["object"] = "list";
  json_resp["data"] = model_array;

  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = 200;
  callback(std::move(status), std::move(json_resp));
  CTL_INF("Running models responded");
}

void RemoteEngine::LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model") || !json_body->isMember("model_path") ||
      !json_body->isMember("api_key") || !json_body->isMember("metadata")) {
    Json::Value error;
    error["error"] =
        "Missing required fields: model, model_path, api_key or metadata";
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

  metadata_ = (*json_body)["metadata"];
  if (!metadata_["transform_req"].isNull() &&
      !metadata_["transform_req"]["chat_completions"].isNull() &&
      !metadata_["transform_req"]["chat_completions"]["template"].isNull()) {
    chat_req_template_ =
        metadata_["transform_req"]["chat_completions"]["template"].asString();
    CTL_INF(chat_req_template_);
  } else {
    CTL_WRN("Required transform_req");
  }

  if (!metadata_["transform_resp"].isNull() &&
      !metadata_["transform_resp"]["chat_completions"].isNull() &&
      !metadata_["transform_resp"]["chat_completions"]["template"].isNull()) {
    chat_res_template_ =
        metadata_["transform_resp"]["chat_completions"]["template"].asString();
    CTL_INF(chat_res_template_);
  } else {
    CTL_WRN("Required transform_resp");
  }

  if (!metadata_["transform_req"].isNull() &&
      !metadata_["transform_req"]["chat_completions"].isNull() &&
      !metadata_["transform_req"]["chat_completions"]["url"].isNull()) {
    chat_url_ =
        metadata_["transform_req"]["chat_completions"]["url"].asString();
    CTL_INF(chat_url_);
  }

  if (!metadata_["header_template"].isNull()) {
    header_ = ReplaceHeaderPlaceholders(metadata_["header_template"].asString(),
                                        *json_body);
    for (auto const& h : header_) {
      CTL_DBG("header: " << h);
    }
  }

  if (!LoadModelConfig(model, model_path, *json_body)) {
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
    // Validate JSON body
    if (!json_body || json_body->isNull()) {
      throw std::runtime_error("Invalid or null JSON body");
    }

    // Get template string with error check
    std::string template_str;
    if (!chat_req_template_.empty()) {
      CTL_DBG("Use engine transform request template: " << chat_req_template_);
      template_str = chat_req_template_;
    } else {
      CTL_WRN("Required transform request template");
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
    q_.runTaskInQueue([this, model_config, result, cb = std::move(callback)] {
      MakeStreamingChatCompletionRequest(*model_config, result, cb);
    });
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
      LOG_WARN << "Failed to parse response: " << response.body;
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
        CTL_WRN("Required transform response template");
      }

      try {
        response_json["stream"] = false;
        if (!response_json.isMember("model")) {
          response_json["model"] = model;
        }
        response_str = renderer_.Render(template_str, response_json);
      } catch (const std::exception& e) {
        throw std::runtime_error("Template rendering error: " +
                                 std::string(e.what()));
      }
    } catch (const std::exception& e) {
      // Log error and potentially rethrow or handle accordingly
      LOG_WARN << "Error: " << e.what();
      LOG_WARN << "Response: " << response.body;
      LOG_WARN << "Using original body";
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
      LOG_WARN << "Failed to parse response: " << response_str;
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

Json::Value RemoteEngine::GetRemoteModels(const std::string& url,
                                          const std::string& api_key,
                                          const std::string& header_template) {
  if (url.empty()) {
    return Json::Value();
  } else {
    auto response = MakeGetModelsRequest(url, api_key, header_template);
    if (response.error) {
      Json::Value error;
      error["error"] = response.error_message;
      CTL_WRN(response.error_message);
      return error;
    }
    CTL_DBG(response.body);
    auto body_json = json_helper::ParseJsonString(response.body);
    if (body_json.isMember("error") && !body_json["error"].isNull()) {
      return body_json["error"];
    }

    // hardcode for cohere
    if (url.find("api.cohere.ai") != std::string::npos) {
      if (body_json.isMember("models")) {
        for (auto& model : body_json["models"]) {
          if (model.isMember("name")) {
            model["id"] = model["name"];
            model.removeMember("name");
          }
        }
        body_json["data"] = body_json["models"];
        body_json.removeMember("models");
      }
    }
    return body_json;
  }
}

}  // namespace remote_engine