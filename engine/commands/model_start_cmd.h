#pragma once
#include <string>
#include "config/model_config.h"

namespace commands {

class ModelStartCmd {
 public:
  bool Exec(const std::string& host, int port, const std::string& model_handle);

  bool Exec(const std::string& host, int port, const config::ModelConfig& mc);
};
}  // namespace commands
