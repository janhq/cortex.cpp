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
  CurlResponse MakeGetModelsRequest();

  // Internal model management
  bool LoadModelConfig(const std::string& model,
                       const std::string& yaml_path,
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