#pragma once

#include <curl/curl.h>
#include <json/json.h>
#include <yaml-cpp/yaml.h>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include "config/model_config.h"
#include "trantor/utils/ConcurrentTaskQueue.h"

#include "cortex-common/EngineI.h"
#include "extensions/template_renderer.h"
#include "utils/file_logger.h"
#include "utils/file_manager_utils.h"
#include "utils/process_status_utils.h"
#include "utils/curl_utils.h"
#include "utils/process/utils.h"

// Helper for CURL response
namespace python_engine {
struct StreamContext {
  std::shared_ptr<std::function<void(Json::Value&&, Json::Value&&)>> callback;
  std::string buffer;
};

struct CurlResponse {
  std::string body;
  bool error{false};
  std::string error_message;
};

class PythonEngine : public EngineI {
 private:
  // Model configuration

  // Thread-safe model config storage
  mutable std::shared_mutex models_mutex_;
  std::unordered_map<std::string, config::PythonModelConfig> models_;
  extensions::TemplateRenderer renderer_;
  std::unique_ptr<trantor::FileLogger> async_file_logger_;
  std::unordered_map<std::string, pid_t> process_map_;
  trantor::ConcurrentTaskQueue q_;

  // Helper functions
  CurlResponse MakePostRequest(const std::string& model,
                               const std::string& path,
                               const std::string& body);
  CurlResponse MakeGetRequest(const std::string& model,
                              const std::string& path);
  CurlResponse MakeDeleteRequest(const std::string& model,
                                 const std::string& path);
  CurlResponse MakeStreamPostRequest(
      const std::string& model, const std::string& path,
      const std::string& body,
      const std::function<void(Json::Value&&, Json::Value&&)>& callback);

  // Process manager functions
  bool TerminateModelProcess(const std::string& model);

  // Internal model management
  bool LoadModelConfig(const std::string& model, const std::string& yaml_path);
  config::PythonModelConfig* GetModelConfig(const std::string& model);

 public:
  PythonEngine();
  ~PythonEngine();

  void Load(EngineLoadOption opts) override;

  void Unload(EngineUnloadOption opts) override;

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
  void HandleRouteRequest(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  void HandleInference(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  Json::Value GetRemoteModels() override;
  void StopInferencing(const std::string& model_id) override;
};
}  // namespace python_engine