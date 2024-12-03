#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include "services/engine_service.h"
#include "utils/result.hpp"

namespace services {
// Status and result
using InferResult = std::pair<Json::Value, Json::Value>;

struct SyncQueue {
  void push(InferResult&& p) {
    std::unique_lock<std::mutex> l(mtx);
    q.push(p);
    cond.notify_one();
  }

  InferResult wait_and_pop() {
    std::unique_lock<std::mutex> l(mtx);
    cond.wait(l, [this] { return !q.empty(); });
    auto res = q.front();
    q.pop();
    return res;
  }

  std::mutex mtx;
  std::condition_variable cond;
  std::queue<InferResult> q;
};

class InferenceService {
 public:
  explicit InferenceService(std::shared_ptr<EngineService> engine_service)
      : engine_service_{engine_service} {}

  cpp::result<void, InferResult> HandleChatCompletion(
      std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body);

  cpp::result<void, InferResult> HandleEmbedding(
      std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body);

  InferResult LoadModel(std::shared_ptr<Json::Value> json_body);

  InferResult UnloadModel(const std::string& engine,
                          const std::string& model_id);

  InferResult GetModelStatus(std::shared_ptr<Json::Value> json_body);

  InferResult GetModels(std::shared_ptr<Json::Value> json_body);

  InferResult FineTuning(std::shared_ptr<Json::Value> json_body);

  bool StopInferencing(const std::string& engine_name,
                       const std::string& model_id);

  bool HasFieldInReq(std::shared_ptr<Json::Value> json_body,
                     const std::string& field);

 private:
  std::shared_ptr<EngineService> engine_service_;
};
}  // namespace services
