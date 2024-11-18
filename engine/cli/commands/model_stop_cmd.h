#pragma once

#include <string>
#include "services/model_service.h"

namespace commands {

class ModelStopCmd {
 public:
  explicit ModelStopCmd(const ModelService& model_service)
      : model_service_{model_service} {};

  void Exec(const std::string& host, int port, const std::string& model_handle);

 private:
  ModelService model_service_;
};
}  // namespace commands
