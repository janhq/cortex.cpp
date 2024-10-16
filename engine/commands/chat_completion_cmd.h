#pragma once

#include <string>
#include <vector>
#include "config/model_config.h"
#include "services/model_service.h"

namespace commands {
class ChatCompletionCmd {
 public:
  explicit ChatCompletionCmd(const ModelService& model_service)
      : model_service_{model_service} {};

  void Exec(const std::string& host, int port, const std::string& model_handle,
            std::string msg);
  void Exec(const std::string& host, int port, const std::string& model_handle,
            const config::ModelConfig& mc, std::string msg);

 private:
  std::vector<Json::Value> histories_;
  ModelService model_service_;
};
}  // namespace commands
