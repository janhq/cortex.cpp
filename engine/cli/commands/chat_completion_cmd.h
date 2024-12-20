#pragma once

#include <string>
#include <vector>
#include "config/model_config.h"

namespace commands {
class ChatCompletionCmd {
 public:
  void Exec(const std::string& host, int port, const std::string& model_handle,
            std::string msg);
  void Exec(const std::string& host, int port, const std::string& model_handle,
            const config::ModelConfig& mc, std::string msg);

 private:
  std::vector<Json::Value> histories_;
};
}  // namespace commands
