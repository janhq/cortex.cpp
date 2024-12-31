#pragma once

#include <string>
#include <vector>
#include "config/model_config.h"
#include "services/database_service.h"

namespace commands {
class ChatCompletionCmd {
 public:
  explicit ChatCompletionCmd(std::shared_ptr<DatabaseService> db_service)
      : db_service_(db_service) {}
  void Exec(const std::string& host, int port, const std::string& model_handle,
            std::string msg);
  void Exec(const std::string& host, int port, const std::string& model_handle,
            const config::ModelConfig& mc, std::string msg);

 private:
  std::shared_ptr<DatabaseService> db_service_;
  std::vector<Json::Value> histories_;
};
}  // namespace commands
