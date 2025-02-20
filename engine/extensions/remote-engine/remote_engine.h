#pragma once

#include <curl/curl.h>
#include <json/json.h>
#include <yaml-cpp/yaml.h>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include "cortex-common/remote_enginei.h"
#include "extensions/template_renderer.h"
#include "trantor/utils/ConcurrentTaskQueue.h"
#include "utils/engine_constants.h"
#include "utils/file_logger.h"
// Helper for CURL response

namespace remote_engine {

struct StreamContext {
  std::shared_ptr<std::function<void(Json::Value&&, Json::Value&&)>> callback;
  std::string buffer;
  // Cache value for Anthropic
  std::string id;
  std::string model;
  extensions::TemplateRenderer& renderer;
  std::string stream_template;
  bool need_stop = true;
  std::string last_request;
  std::string chunks;
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
  extensions::TemplateRenderer renderer_;
  Json::Value metadata_;
  std::string chat_req_template_;
  std::string chat_res_template_;
  std::vector<std::string> header_;
  std::string engine_name_;
  std::string chat_url_;
  trantor::ConcurrentTaskQueue q_;

  // Helper functions
  CurlResponse MakeChatCompletionRequest(const ModelConfig& config,
                                         const std::string& body,
                                         const std::string& method = "POST");
  CurlResponse MakeStreamingChatCompletionRequest(
      const ModelConfig& config, const std::string& body,
      const std::function<void(Json::Value&&, Json::Value&&)>& callback);
  CurlResponse MakeGetModelsRequest(const std::string& url,
                                    const std::string& api_key,
                                    const std::string& header_template);

  // Internal model management
  bool LoadModelConfig(const std::string& model, const std::string& yaml_path,
                       const Json::Value& body);
  ModelConfig* GetModelConfig(const std::string& model);

 public:
  explicit RemoteEngine(const std::string& engine_name);
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

  Json::Value GetRemoteModels(const std::string& url,
                              const std::string& api_key,
                              const std::string& header_template) override;
};

}  // namespace remote_engine