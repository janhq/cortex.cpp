#pragma once

#include "common/cortex/sync_queue.h"
#include "services/engine_service.h"
#include "services/model_service.h"
#include "utils/result.hpp"

class InferenceService {
 public:
  explicit InferenceService(std::shared_ptr<EngineService> engine_service)
      : engine_service_{engine_service} {}

  cpp::result<void, InferResult> HandleChatCompletion(
      std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body);

  cpp::result<void, InferResult> HandleEmbedding(
      std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body);

  cpp::result<void, InferResult> HandleInference(
      std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body);

  cpp::result<void, InferResult> HandleRouteRequest(
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

  void SetModelService(std::shared_ptr<ModelService> model_service) {
    model_service_ = model_service;
  }

 private:
  std::shared_ptr<EngineService> engine_service_;
  std::weak_ptr<ModelService> model_service_;
};
