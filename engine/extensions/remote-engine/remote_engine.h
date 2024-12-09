#pragma once

#include <curl/curl.h>
#include <json/json.h>
#include <yaml-cpp/yaml.h>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include "cortex-common/remote_enginei.h"
#include "extensions/remote-engine/template_renderer.h"
#include "utils/engine_constants.h"
#include "utils/file_logger.h"
// Helper for CURL response

namespace remote_engine {
inline bool IsRemoteEngine(std::string_view e) {
  return e == kAnthropicEngine || e == kOpenAiEngine;
}


struct StreamContext {
  std::shared_ptr<std::function<void(Json::Value&&, Json::Value&&)>> callback;
  std::string buffer;
  // Cache value for Anthropic
  std::string id;
  std::string model;
  TemplateRenderer& renderer;
  std::string stream_template;
};
struct CurlResponse {
  std::string body;
  bool error{false};
  std::string error_message;
};

class RemoteEngine : public RemoteEngineI {
 protected:
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
  mutable std::shared_mutex models_mtx_;
  std::unordered_map<std::string, ModelConfig> models_;
  TemplateRenderer renderer_;
  Json::Value metadata_;
  std::string api_key_template_;

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
  virtual ~RemoteEngine();

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
  
  Json::Value GetRemoteModels() override;
};

}  // namespace remote_engine