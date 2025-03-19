#pragma once

#include <filesystem>
#include <functional>
#include <memory>

#include "json/value.h"
#include "trantor/utils/Logger.h"
class EngineI {
 public:
  struct EngineLoadOption {
    // engine
    std::filesystem::path engine_path;
    std::filesystem::path deps_path;
    bool is_custom_engine_path;

    // logging
    std::filesystem::path log_path;
    int max_log_lines;
    trantor::Logger::LogLevel log_level;
  };

  struct EngineUnloadOption {
    // place holder for now
  };

  virtual ~EngineI() {}

  virtual void Load(EngineLoadOption opts) = 0;

  virtual void Unload(EngineUnloadOption opts) = 0;

  // cortex.llamacpp interface
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

  virtual bool SetFileLogger(int max_log_lines,
                             const std::string& log_path) = 0;
  virtual void SetLogLevel(trantor::Logger::LogLevel logLevel) = 0;

  // Stop inflight chat completion in stream mode
  virtual void StopInferencing(const std::string& model_id) = 0;
};
