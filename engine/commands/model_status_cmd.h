#pragma once
#include <string>
#include "config/yaml_config.h"

namespace commands {

class ModelStatusCmd {
 public:
  bool IsLoaded(const std::string& host, int port,
                const config::ModelConfig& mc);
};
}  // namespace commands