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
#ifdef _WIN32
#include <process.h>
#include <windows.h>
using pid_t = DWORD;
#elif __APPLE__ || __linux__
#include <signal.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
// Helper for CURL response
namespace python_engine {
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
    LOG_INFO << "line: "<<line;
    // Skip empty lines
    if (line.empty() || line == "\r")
      continue;


    // Skip [DONE] message

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

class PythonEngine : public EngineI {
 private:
  // Model configuration

  // Thread-safe model config storage
  mutable std::shared_mutex models_mutex_;
  std::unordered_map<std::string, config::PythonModelConfig> models_;
  extensions::TemplateRenderer renderer_;
  std::unique_ptr<trantor::FileLogger> async_file_logger_;
  std::unordered_map<std::string, pid_t> processMap;
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
  pid_t SpawnProcess(const std::string& model,
                     const std::vector<std::string>& command);
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