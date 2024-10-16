#pragma once

#include <string>
#include "services/model_service.h"

namespace commands {

class ModelDelCmd {
 public:
  explicit ModelDelCmd(std::shared_ptr<DownloadService> download_service)
      : model_service_{ModelService(download_service)} {};

  void Exec(const std::string& model_handle);

 private:
  ModelService model_service_;
};
}  // namespace commands
