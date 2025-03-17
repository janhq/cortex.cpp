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

  static cpp::result<void, std::string> Download(
      std::shared_ptr<DownloadService>& download_service,
      const std::string& version,
      const std::optional<std::string> variant_name);

  virtual void Load(EngineLoadOption opts) override;
  virtual void Unload(EngineUnloadOption opts) override;

  // cortex.llamacpp interface
  virtual void HandleChatCompletion(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  virtual void HandleEmbedding(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  virtual void LoadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  virtual void UnloadModel(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  virtual void GetModelStatus(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  // For backward compatible checking
  virtual bool IsSupported(const std::string& f) override;

  // Get list of running models
  virtual void GetModels(
      std::shared_ptr<Json::Value> jsonBody,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  virtual bool SetFileLogger(int max_log_lines,
                             const std::string& log_path) override;
  virtual void SetLogLevel(trantor::Logger::LogLevel logLevel) override;

  // Stop inflight chat completion in stream mode
  virtual void StopInferencing(const std::string& model_id) override;

  virtual Json::Value GetRemoteModels() override;
  virtual void HandleRouteRequest(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
  virtual void HandleInference(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
};
