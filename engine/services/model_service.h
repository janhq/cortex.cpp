#pragma once

#include <memory>
#include <string>
#include "config/model_config.h"
#include "services/download_service.h"

class ModelService {
 public:
  constexpr auto static kHuggingFaceHost = "huggingface.co";

  explicit ModelService(std::shared_ptr<DownloadService> download_service)
      : download_service_{download_service} {};

  /**
   * Return model id if download successfully
   */
  cpp::result<std::string, std::string> DownloadModel(const std::string& input);

  cpp::result<void, std::string> AbortDownloadModel(const std::string& task_id);

  cpp::result<std::string, std::string> DownloadModelFromCortexso(
      const std::string& name, const std::string& branch = "main");

  cpp::result<DownloadTask, std::string> DownloadModelFromCortexsoAsync(
      const std::string& name, const std::string& branch = "main");

  std::optional<config::ModelConfig> GetDownloadedModel(
      const std::string& modelId) const;

  /**
   * Delete a model from local. If a model is an import model, we only delete
   * in our database/model.list.
   */
  cpp::result<void, std::string> DeleteModel(const std::string& model_handle);

  cpp::result<bool, std::string> StartModel(const std::string& host, int port,
                                            const std::string& model_handle);

  cpp::result<bool, std::string> StopModel(const std::string& host, int port,
                                           const std::string& model_handle);

  cpp::result<bool, std::string> GetModelStatus(
      const std::string& host, int port, const std::string& model_handle);

  cpp::result<std::string, std::string> HandleUrl(const std::string& url,
                                                  bool async = false);

  cpp::result<DownloadTask, std::string> HandleDownloadUrlAsync(
      const std::string& url);

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
};
