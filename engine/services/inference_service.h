#pragma once

#include <optional>
#include <variant>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "cortex-common/EngineI.h"
#include "cortex-common/cortexpythoni.h"
#include "utils/dylib.h"
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
  cpp::result<void, InferResult> HandleChatCompletion(
      std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body);

  cpp::result<void, InferResult> HandleEmbedding(
      std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body);

  InferResult LoadModel(std::shared_ptr<Json::Value> json_body);

  InferResult UnloadModel(std::shared_ptr<Json::Value> json_body);

  InferResult GetModelStatus(std::shared_ptr<Json::Value> json_body);

  InferResult GetModels(std::shared_ptr<Json::Value> json_body);

  Json::Value GetEngines(std::shared_ptr<Json::Value> json_body);

  InferResult FineTuning(std::shared_ptr<Json::Value> json_body);

  InferResult UnloadEngine(std::shared_ptr<Json::Value> json_body);

 private:
  bool IsEngineLoaded(const std::string& e);

  bool HasFieldInReq(std::shared_ptr<Json::Value> json_body,
                     const std::string& field);

 private:
  using EngineV = std::variant<EngineI*, CortexPythonEngineI*>;
  struct EngineInfo {
    std::unique_ptr<cortex_cpp::dylib> dl;
    EngineV engine;
#if defined(_WIN32)
    DLL_DIRECTORY_COOKIE cookie;
#endif
  };
  // TODO(sang) move engines_ into engine service?
  std::unordered_map<std::string, EngineInfo> engines_;
};
}  // namespace services