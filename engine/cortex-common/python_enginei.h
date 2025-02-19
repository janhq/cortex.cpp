#pragma once

#include <functional>
#include <memory>

#include "json/value.h"

class PythonEngineI {
 public:
  virtual ~PythonEngineI() {}

  // virtual bool IsSupported(const std::string& f) = 0;

  // model management
  virtual void LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void UnloadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void GetModelStatus(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void GetModels(
    std::shared_ptr<Json::Value> jsonBody,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;

  virtual void HandleRequest(
      const std::string& model,
      const std::vector<std::string>& path_parts,
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
};
