#pragma once

#include <memory>
#include <optional>
#include <string>
#include "config/model_config.h"
#include "services/download_service.h"
#include "services/inference_service.h"

struct StartParameterOverride {
std::optional<bool> cache_enabled;
std::optional<int> ngl;
std::optional<int> n_parallel;
std::optional<int> ctx_len;
std::optional<std::string> custom_prompt_template;
std::optional<std::string> cache_type;
};
class ModelService {
 public:
  constexpr auto static kHuggingFaceHost = "huggingface.co";

  explicit ModelService(std::shared_ptr<DownloadService> download_service)
      : download_service_{download_service} {};

  explicit ModelService(
      std::shared_ptr<DownloadService> download_service,
      std::shared_ptr<services::InferenceService> inference_service)
      : download_service_{download_service},
        inference_svc_(inference_service) {};

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

  cpp::result<bool, std::string> StartModel(
      const std::string& host, int port, const std::string& model_handle,
      const StartParameterOverride& params_override);

  cpp::result<bool, std::string> StopModel(const std::string& host, int port,
                                           const std::string& model_handle);

  cpp::result<bool, std::string> GetModelStatus(
      const std::string& host, int port, const std::string& model_handle);

  cpp::result<std::string, std::string> HandleUrl(const std::string& url);

  cpp::result<DownloadTask, std::string> HandleDownloadUrlAsync(
      const std::string& url, std::optional<std::string> temp_model_id);

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

  std::shared_ptr<DownloadService> download_service_;
  std::shared_ptr<services::InferenceService> inference_svc_;
};
