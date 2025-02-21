#pragma once

#include <json/json.h>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "cortex-common/python_enginei.h"
#include "services/download_service.h"
#include "utils/process/utils.h"

namespace python_engine {

// UV-related functions
cpp::result<void, std::string> DownloadUv(std::shared_ptr<DownloadService>& download_service);
std::string GetUvPath();
bool IsUvInstalled();

class PythonEngine : public PythonEngineI {
 private:
  struct PythonSubprocess {
    pid_t pid;
    int port;
    uint64_t start_time;

    bool IsAlive();
    bool Kill();
  };

  mutable std::shared_mutex mutex;
  std::unordered_map<std::string, PythonSubprocess> model_process_map;

 public:
  PythonEngine();
  ~PythonEngine();

  void LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  void UnloadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  void GetModelStatus(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  void GetModels(
    std::shared_ptr<Json::Value> jsonBody,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  cpp::result<int, std::string> GetPort(const std::string& model) override;
};
}  // namespace python_engine
