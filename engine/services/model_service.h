#pragma once

#include <memory>
#include <optional>
#include <string>
#include "common/engine_servicei.h"
#include "config/model_config.h"
#include "services/download_service.h"
#include "services/inference_service.h"
#include "utils/hardware/gguf/gguf_file_estimate.h"

struct ModelPullInfo {
  std::string id;
  std::string default_branch;
  std::vector<std::string> downloaded_models;
  std::vector<std::string> available_models;
  std::string model_source;
  std::string download_url;
};

struct StartParameterOverride {
  std::optional<bool> cache_enabled;
  std::optional<int> ngl;
  std::optional<int> n_parallel;
  std::optional<int> ctx_len;
  std::optional<std::string> custom_prompt_template;
  std::optional<std::string> cache_type;
  std::optional<std::string> mmproj;
  std::optional<std::string> model_path;
  bool bypass_llama_model_path = false;
  bool bypass_model_check() const {
    return mmproj.has_value() || bypass_llama_model_path;
  }
};

struct StartModelResult {
  bool success;
  std::optional<std::string> warning;
};

class ModelService {
 public:
  void ForceIndexingModelList();

  explicit ModelService(std::shared_ptr<DownloadService> download_service)
      : download_service_{download_service} {};

  explicit ModelService(
      std::shared_ptr<DownloadService> download_service,
      std::shared_ptr<services::InferenceService> inference_service,
      std::shared_ptr<EngineServiceI> engine_svc)
      : download_service_{download_service},
        inference_svc_(inference_service),
        engine_svc_(engine_svc) {};

  /**
   * Return model id if download successfully
   */
  cpp::result<std::string, std::string> DownloadModel(const std::string& input);

  cpp::result<std::string, std::string> AbortDownloadModel(
      const std::string& task_id);

  cpp::result<std::string, std::string> DownloadModelFromCortexso(
      const std::string& name, const std::string& branch = "main");

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
      const std::string& model_handle,
      const StartParameterOverride& params_override);

  cpp::result<bool, std::string> StopModel(const std::string& model_handle);

  cpp::result<bool, std::string> GetModelStatus(
      const std::string& model_handle);

  cpp::result<ModelPullInfo, std::string> GetModelPullInfo(
      const std::string& model_handle);

  cpp::result<std::string, std::string> HandleUrl(const std::string& url);

  cpp::result<DownloadTask, std::string> HandleDownloadUrlAsync(
      const std::string& url, std::optional<std::string> temp_model_id,
      std::optional<std::string> temp_name);

  bool HasModel(const std::string& id) const;

  cpp::result<std::optional<hardware::Estimation>, std::string> GetEstimation(
      const std::string& model_handle, const std::string& kv_cache = "f16",
      int n_batch = 2048, int n_ubatch = 2048);

 private:
  /**
   * Handle downloading model which have following pattern: author/model_name
   */
  cpp::result<std::string, std::string> DownloadHuggingFaceGgufModel(
      const std::string& author, const std::string& modelName,
      std::optional<std::string> fileName);

  /**
   * Handling cortexso models. Will look through cortexso's HF repository and
   * listing all the branches, except main. Then print out the selection for user.
   */
  cpp::result<std::string, std::string> HandleCortexsoModel(
      const std::string& modelName);

  cpp::result<std::optional<std::string>, std::string> MayFallbackToCpu(
      const std::string& model_path, int ngl, int ctx_len, int n_batch = 2048,
      int n_ubatch = 2048, const std::string& kv_cache_type = "f16");

  std::shared_ptr<DownloadService> download_service_;
  std::shared_ptr<services::InferenceService> inference_svc_;
  std::unordered_set<std::string> bypass_stop_check_set_;
  std::shared_ptr<EngineServiceI> engine_svc_ = nullptr;
};
