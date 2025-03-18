#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "cortex-common/EngineI.h"
#include "json/json.h"
#include "services/engine_service.h"
#include "utils/process/utils.h"
#include "utils/task_queue.h"

namespace cortex::local {
using http_callback = std::function<void(Json::Value&&, Json::Value&&)>;

struct ServerAddress {
  std::string host;
  int port;
  cortex::process::ProcessInfo process_info;
  std::string pre_prompt;
  std::string user_prompt;
  std::string ai_prompt;
  std::string system_prompt;
};
class LocalEngine : public EngineI {
 public:
  LocalEngine(EngineService& engine_service, TaskQueue& q)
      : engine_service_(engine_service), q_(q) {}
  ~LocalEngine();

  void Load(EngineLoadOption opts) final {}

  void Unload(EngineUnloadOption opts) final {}

  void HandleChatCompletion(std::shared_ptr<Json::Value> json_body,
                            http_callback&& callback) final;
  void HandleEmbedding(std::shared_ptr<Json::Value> json_body,
                       http_callback&& callback) final;
  void LoadModel(std::shared_ptr<Json::Value> json_body,
                 http_callback&& callback) final;
  void UnloadModel(std::shared_ptr<Json::Value> json_body,
                   http_callback&& callback) final;
  void GetModelStatus(std::shared_ptr<Json::Value> json_body,
                      http_callback&& callback) final;

  // Get list of running models
  void GetModels(std::shared_ptr<Json::Value> jsonBody,
                 http_callback&& callback) final;

  bool SetFileLogger(int max_log_lines, const std::string& log_path) final {
    return true;
  }
  void SetLogLevel(trantor::Logger::LogLevel logLevel) final {}

  // Stop inflight chat completion in stream mode
  void StopInferencing(const std::string& model_id) final {}

  void HandleRouteRequest(std::shared_ptr<Json::Value> json_body,
                          http_callback&& callback) final {}

  void HandleInference(std::shared_ptr<Json::Value> json_body,
                       http_callback&& callback) final {}

 private:
  void HandleOpenAiChatCompletion(std::shared_ptr<Json::Value> json_body,
                                  http_callback&& callback,
                                  const std::string& model);

  void HandleNonOpenAiChatCompletion(std::shared_ptr<Json::Value> json_body,
                                     http_callback&& callback,
                                     const std::string& model);

 private:
  std::unordered_map<std::string, ServerAddress> server_map_;
  EngineService& engine_service_;
  TaskQueue& q_;
};

}  // namespace cortex::local