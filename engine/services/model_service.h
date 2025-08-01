#pragma once

#include <memory>
#include <optional>
#include <string>
#include "common/engine_servicei.h"
#include "common/model_metadata.h"
#include "config/model_config.h"
#include "services/database_service.h"
#include "services/download_service.h"
#include "services/hardware_service.h"
#include "utils/hardware/gguf/gguf_file_estimate.h"
#include "utils/task_queue.h"

class InferenceService;

struct ModelPullInfo {
  std::string id;
  std::string default_branch;
  std::vector<std::string> downloaded_models;
  std::vector<std::string> available_models;
  std::string model_source;
  std::string download_url;
};

struct StartModelResult {
  bool success;
  std::optional<std::string> warning;
};

class ModelService {
 public:
  void ForceIndexingModelList();

  explicit ModelService(std::shared_ptr<DatabaseService> db_service,
                        std::shared_ptr<HardwareService> hw_service,
                        std::shared_ptr<DownloadService> download_service,
                        std::shared_ptr<InferenceService> inference_service,
                        std::shared_ptr<EngineServiceI> engine_svc,
                        cortex::TaskQueue& task_queue);

  cpp::result<std::string, std::string> AbortDownloadModel(
      const std::string& task_id);

  cpp::result<DownloadTask, std::string> DownloadModelFromCortexsoAsync(
      const std::string& name, const std::string& branch = "main",
      std::optional<std::string> temp_model_id = std::nullopt);

  std::optional<config::ModelConfig> GetDownloadedModel(
      const std::string& modelId) const;

  /**
   * Delete a model from local. If a model is an import model, we only delete
   * in our database/model.list.
   */
  cpp::result<void, std::string> DeleteModel(const std::string& model_handle);

  cpp::result<StartModelResult, std::string> StartModel(
      const std::string& model_handle, const Json::Value& params_override,
      bool bypass_model_check);

  cpp::result<bool, std::string> StopModel(const std::string& model_handle);

  cpp::result<bool, std::string> GetModelStatus(
      const std::string& model_handle);

  cpp::result<ModelPullInfo, std::string> GetModelPullInfo(
      const std::string& model_handle);

  cpp::result<DownloadTask, std::string> HandleDownloadUrlAsync(
      const std::string& url, std::optional<std::string> temp_model_id,
      std::optional<std::string> temp_name);

  bool HasModel(const std::string& id) const;

  std::optional<hardware::Estimation> GetEstimation(
      const std::string& model_handle);

  cpp::result<std::optional<hardware::Estimation>, std::string> EstimateModel(
      const std::string& model_handle, const std::string& kv_cache = "f16",
      int n_batch = 2048, int n_ubatch = 2048);

  cpp::result<std::shared_ptr<ModelMetadata>, std::string> GetModelMetadata(
      const std::string& model_id) const;

  std::string GetEngineByModelId(const std::string& model_id) const;

 private:
  cpp::result<std::optional<std::string>, std::string> MayFallbackToCpu(
      const std::string& model_path, int ngl, int ctx_len, int n_batch = 2048,
      int n_ubatch = 2048, const std::string& kv_cache_type = "f16");

  void ProcessBgrTasks();

  int GetCpuThreads() const;

  std::shared_ptr<DatabaseService> db_service_;
  std::shared_ptr<HardwareService> hw_service_;
  std::shared_ptr<DownloadService> download_service_;
  std::shared_ptr<InferenceService> inference_svc_;
  std::unordered_set<std::string> bypass_stop_check_set_;
  std::shared_ptr<EngineServiceI> engine_svc_ = nullptr;

  std::mutex es_mtx_;
  std::unordered_map<std::string, std::optional<hardware::Estimation>> es_;
  cortex::TaskQueue& task_queue_;
};
