#include <functional>
#include "common/engine_servicei.h"
#include "cortex-common/EngineI.h"
#include "python_utils.h"

class VllmEngine : public EngineI {
 private:
  mutable std::shared_mutex mutex;
  std::unordered_map<std::string, python_utils::PythonSubprocess>
      model_process_map;

 public:
  VllmEngine() {};
  ~VllmEngine();

  static std::vector<EngineVariantResponse> GetVariants();

  void Load(EngineLoadOption opts) override;
  void Unload(EngineUnloadOption opts) override;

  // cortex.llamacpp interface
  void HandleChatCompletion(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  void HandleEmbedding(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  void LoadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  void UnloadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  void GetModelStatus(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  // For backward compatible checking
  bool IsSupported(const std::string& f) override;

  // Get list of running models
  void GetModels(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  bool SetFileLogger(int max_log_lines, const std::string& log_path) override;
  void SetLogLevel(trantor::Logger::LogLevel logLevel) override;

  // Stop inflight chat completion in stream mode
  void StopInferencing(const std::string& model_id) override;
};
