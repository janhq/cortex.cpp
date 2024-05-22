#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include "json/value.h"

class EngineI {
 public:
  virtual ~EngineI() {}

  virtual void HandleChatCompletion(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void HandleEmbedding(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void LoadModel(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void UnloadModel(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void GetModelStatus(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;

  // For backward compatible checking
  virtual bool IsSupported(const std::string& f) = 0;

  // Get list of running models
  virtual void GetModels(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
};
