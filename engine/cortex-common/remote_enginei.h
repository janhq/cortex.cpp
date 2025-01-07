#pragma once

#pragma once

#include <functional>
#include <memory>

#include "json/value.h"
#include "trantor/utils/Logger.h"
class RemoteEngineI {
 public:
  virtual ~RemoteEngineI() {}

  virtual void HandleChatCompletion(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void HandleEmbedding(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void LoadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void UnloadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void GetModelStatus(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;

  // Get list of running models
  virtual void GetModels(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;

  // Get available remote models
  virtual Json::Value GetRemoteModels(const std::string& url,
                                      const std::string& api_key,
                                      const std::string& header_template) = 0;
};
