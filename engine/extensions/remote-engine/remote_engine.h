#pragma once

#include <curl/curl.h>
#include <json/json.h>
#include <yaml-cpp/yaml.h>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include "cortex-common/EngineI.h"
#include "extensions/remote-engine/TemplateRenderer.h"
#include "utils/file_logger.h"
// Helper for CURL response

namespace remote_engine {
struct StreamContext {
  std::shared_ptr<std::function<void(Json::Value&&, Json::Value&&)>> callback;
  std::string buffer;
};

static size_t StreamWriteCallback(char* ptr, size_t size, size_t nmemb,
                                  void* userdata) {
  auto* context = static_cast<StreamContext*>(userdata);
  std::string chunk(ptr, size * nmemb);

  context->buffer += chunk;

  // Process complete lines
  size_t pos;
  while ((pos = context->buffer.find('\n')) != std::string::npos) {
    std::string line = context->buffer.substr(0, pos);
    context->buffer = context->buffer.substr(pos + 1);

    // Skip empty lines
    if (line.empty() || line == "\r")
      continue;

    // Remove "data: " prefix if present
    // if (line.substr(0, 6) == "data: ")
    // {
    //     line = line.substr(6);
    // }

    // Skip [DONE] message
    std::cout << line << std::endl;
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

struct CurlResponse {
  std::string body;
  bool error{false};
  std::string error_message;
};

class RemoteEngine : public EngineI {
 private:
  // Model configuration
  struct ModelConfig {
    std::string model;
    std::string version;
    std::string api_key;
    std::string url;
    YAML::Node transform_req;
    YAML::Node transform_resp;
    bool is_loaded{false};
  };

  // Thread-safe model config storage
  mutable std::shared_mutex models_mutex_;
  std::unordered_map<std::string, ModelConfig> models_;
  TemplateRenderer renderer_;
  Json::Value metadata_;
  std::string api_key_template_;
  std::unique_ptr<trantor::FileLogger> async_file_logger_;

  // Helper functions
  CurlResponse MakeChatCompletionRequest(const ModelConfig& config,
                                         const std::string& body,
                                         const std::string& method = "POST");
  CurlResponse MakeStreamingChatCompletionRequest(
      const ModelConfig& config, const std::string& body,
      const std::function<void(Json::Value&&, Json::Value&&)>& callback);
  CurlResponse MakeGetModelsRequest();

  // Internal model management
  bool LoadModelConfig(const std::string& model, const std::string& yaml_path,
                       const std::string& api_key);
  ModelConfig* GetModelConfig(const std::string& model);

 public:
  RemoteEngine();
  ~RemoteEngine();

  // Main interface implementations
  void GetModels(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  void HandleChatCompletion(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  void LoadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  void UnloadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  void GetModelStatus(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  // Other required virtual functions
  void HandleEmbedding(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  bool IsSupported(const std::string& feature) override;
  bool SetFileLogger(int max_log_lines, const std::string& log_path) override;
  void SetLogLevel(trantor::Logger::LogLevel logLevel) override;
};

}  // namespace remote_engine