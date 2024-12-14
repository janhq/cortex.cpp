#pragma once

#include <filesystem>
#include <functional>
#include <memory>

#include "json/value.h"
#include "trantor/utils/Logger.h"
class EngineI {
 public:
  struct RegisterLibraryOption {
    std::vector<std::filesystem::path> paths;
  };

  struct EngineLoadOption {
    // engine
    std::filesystem::path engine_path;
    std::filesystem::path cuda_path;
    bool custom_engine_path;

    // logging
    std::filesystem::path log_path;
    int max_log_lines;
    trantor::Logger::LogLevel log_level;
  };

  struct EngineUnloadOption {
    bool unload_dll;
  };

  virtual ~EngineI() {}

  /**
   * Being called before starting process to register dependencies search paths.
   */
  virtual void RegisterLibraryPath(RegisterLibraryOption opts) = 0;

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

  // For backward compatible checking
  virtual bool IsSupported(const std::string& f) = 0;

  // Get list of running models
  virtual void GetModels(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;

  virtual bool SetFileLogger(int max_log_lines,
                             const std::string& log_path) = 0;
  virtual void SetLogLevel(trantor::Logger::LogLevel logLevel) = 0;

  virtual Json::Value GetRemoteModels() = 0;
  virtual void HandleRouteRequest(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;
  virtual void HandleInference(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) = 0;

  // Stop inflight chat completion in stream mode
  virtual void StopInferencing(const std::string& model_id) = 0;
};
