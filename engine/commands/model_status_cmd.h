#pragma once
#include <string>
#include "services/model_service.h"

namespace commands {

class ModelStatusCmd {
 public:
  explicit ModelStatusCmd(const ModelService& model_service)
      : model_service_{model_service} {};

  bool IsLoaded(const std::string& host, int port,
                const std::string& model_handle);

 private:
  ModelService model_service_;
};
}  // namespace commands
