#pragma once

#include <curl/curl.h>
#include <json/json.h>
#include <yaml-cpp/yaml.h>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include "config/model_config.h"
#include "trantor/utils/ConcurrentTaskQueue.h"

#include "cortex-common/python_enginei.h"
#include "extensions/template_renderer.h"
#include "utils/file_logger.h"
#include "utils/file_manager_utils.h"
#include "utils/process_status_utils.h"
#include "utils/curl_utils.h"
#include "utils/process/utils.h"
#include "services/download_service.h"

// Helper for CURL response
namespace python_engine {
struct StreamContext {
  std::shared_ptr<std::function<void(Json::Value&&, Json::Value&&)>> callback;
  std::string buffer;
};

struct CurlResponse {
  std::string body;
  bool error{false};
  std::string error_message;
};

// UV-related functions
cpp::result<void, std::string> DownloadUv(std::shared_ptr<DownloadService>& download_service);
std::string GetUvPath();
bool IsUvInstalled();

class PythonEngine : public PythonEngineI {
 private:
  // Model configuration

  // Thread-safe model config storage
  mutable std::shared_mutex models_mutex_;
  std::unordered_map<std::string, config::PythonModelConfig> models_;
  extensions::TemplateRenderer renderer_;
  std::unique_ptr<trantor::FileLogger> async_file_logger_;
  std::unordered_map<std::string, pid_t> process_map_;
  trantor::ConcurrentTaskQueue q_;

 public:
  PythonEngine();
  ~PythonEngine();

  void LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  void HandleRequest(
      const std::string& model,
      const std::vector<std::string>& path_parts,
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
};
}  // namespace python_engine
