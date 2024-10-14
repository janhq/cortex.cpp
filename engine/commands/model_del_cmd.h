#pragma once

#include <string>
#include "services/model_service.h"

namespace commands {

class ModelDelCmd {
 public:
  explicit ModelDelCmd()
      : model_service_{ModelService(std::make_shared<DownloadService>())} {};

  void Exec(const std::string& host, int port, const std::string& model_handle);

 private:
  ModelService model_service_;
};
}  // namespace commands
