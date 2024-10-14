#pragma once

#include "services/model_service.h"

namespace commands {

class ModelPullCmd {
 public:
  explicit ModelPullCmd()
      : model_service_{ModelService(std::make_shared<DownloadService>())} {};
  void Exec(const std::string& input);

 private:
  ModelService model_service_;
};
}  // namespace commands
