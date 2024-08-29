#pragma once
#include <string>
#include <vector>
#include "config/model_config.h"
#include "nlohmann/json.hpp"

namespace commands {
class ChatCmd {
 public:
  ChatCmd(std::string host, int port, const config::ModelConfig& mc);
  void Exec(std::string msg);

 private:
  std::string host_;
  int port_;
  const config::ModelConfig& mc_;
  std::vector<nlohmann::json> histories_;
};
}  // namespace commands