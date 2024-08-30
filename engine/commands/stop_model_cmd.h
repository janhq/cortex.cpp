#pragma once
#include <string>
#include <optional>
#include "config/model_config.h"

namespace commands {

class StopModelCmd{
 public:
  StopModelCmd(std::string host, int port, const config::ModelConfig& mc);
  void Exec();

 private:
  std::string host_;
  int port_;
  const config::ModelConfig& mc_;
};
}  // namespace commands