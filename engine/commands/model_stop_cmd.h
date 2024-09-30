#pragma once

#include <string>
#include "config/model_config.h"

namespace commands {

class ModelStopCmd {
 public:
  void Exec(const std::string& host, int port, const std::string& model_handle);
};
}  // namespace commands
