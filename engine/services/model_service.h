#pragma once

#include <string>
#include "config/model_config.h"
#include "services/download_service.h"

class ModelService {
 public:
  ModelService() : download_service_{DownloadService()} {};

  void DownloadModel(const std::string& input);

  std::optional<config::ModelConfig> GetDownloadedModel(
      const std::string& modelId) const;

 private:
  void DownloadModelByDirectUrl(const std::string& url);

  void DownloadModelFromCortexso(const std::string& name,
                                 const std::string& branch);

  DownloadService download_service_;

  constexpr auto static kHuggingFaceHost = "huggingface.co";
};
