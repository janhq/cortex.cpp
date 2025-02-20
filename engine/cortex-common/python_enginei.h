#pragma once

#include <functional>

#include "json/value.h"
#include "utils/result.hpp"

class PythonEngineI {
 public:
  virtual ~PythonEngineI() {}

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

  virtual cpp::result<int, std::string> GetPort(const std::string& model) = 0;
};
