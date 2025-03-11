#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "cortex-common/local_enginei.h"
#include "json/json.h"
#include "services/engine_service.h"
#include "utils/process/utils.h"
#include "utils/task_queue.h"

namespace cortex::local {
struct ServerAddress {
  std::string host;
  int port;
  cortex::process::ProcessInfo process_info;
};
class LocalEngine : public LocalEngineI {
 public:
  LocalEngine(EngineService& engine_service, TaskQueue& q)
      : engine_service_(engine_service), q_(q) {}
  ~LocalEngine();
  void HandleChatCompletion(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) final;
  void HandleEmbedding(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) final;
  void LoadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) final;
  void UnloadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) final;
  void GetModelStatus(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) final;

  // Get list of running models
  void GetModels(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) final;

 private:
  std::unordered_map<std::string, ServerAddress> server_map_;
  EngineService& engine_service_;
  TaskQueue& q_;
};

}  // namespace cortex::local