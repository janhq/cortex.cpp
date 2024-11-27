#pragma once
#include <string>
#include <unordered_map>
#include "services/model_service.h"

namespace commands {

class ModelStartCmd {
 public:
  explicit ModelStartCmd(const ModelService& model_service)
      : model_service_{model_service} {};

  bool Exec(const std::string& host, int port, const std::string& model_handle,
            const std::unordered_map<std::string, std::string>& options,
            bool print_success_log = true);

 private:
  ModelService model_service_;
};
}  // namespace commands
