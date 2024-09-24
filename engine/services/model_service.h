#pragma once

#include <string>
#include "config/model_config.h"
#include "services/download_service.h"

class ModelService {
 public:
  ModelService() : download_service_{DownloadService()} {};

  /**
   * Return model id if download successfully
   */
  std::optional<std::string> DownloadModel(const std::string& input);

  std::optional<config::ModelConfig> GetDownloadedModel(
      const std::string& modelId) const;

 private:
  std::optional<std::string> DownloadModelByDirectUrl(const std::string& url);

  std::optional<std::string> DownloadModelFromCortexso(
      const std::string& name, const std::string& branch = "main");

  /**
   * Handle downloading model which have following pattern: author/model_name
   */
  std::optional<std::string> DownloadHuggingFaceGgufModel(
      const std::string& author, const std::string& modelName,
      std::optional<std::string> fileName);

  std::optional<std::string> DownloadModelByModelName(
      const std::string& modelName);

  DownloadService download_service_;

  void ParseGguf(const DownloadItem& ggufDownloadItem,
                 std::optional<std::string> author = nullptr) const;

  constexpr auto static kHuggingFaceHost = "huggingface.co";
};
