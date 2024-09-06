#pragma once
#include <string>
#include "config/model_config.h"

namespace commands {

class ModelStartCmd {
 public:
  explicit ModelStartCmd(std::string host, int port,
                         const config::ModelConfig& mc);
  bool Exec();

 private:
  std::string host_;
  int port_;
  const config::ModelConfig& mc_;
};
}  // namespace commands
