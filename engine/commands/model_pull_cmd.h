#pragma once

#include "services/model_service.h"

namespace commands {

class ModelPullCmd {
 public:
  explicit ModelPullCmd(std::shared_ptr<DownloadService> download_service)
      : model_service_{ModelService(download_service)} {};
  void Exec(const std::string& input);

 private:
  ModelService model_service_;
};
}  // namespace commands
