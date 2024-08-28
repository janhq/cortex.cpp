#pragma once
#include <string>
#include <vector>
#include "config/model_config.h"
#include "nlohmann/json.hpp"

namespace commands {
class RunCmd {
 public:
  explicit RunCmd(std::string host, int port, std::string model_id);
  void Exec();

 private:
  bool IsModelExisted(const std::string& model_id);
  bool IsEngineExisted(const std::string& e);

 private:
  std::string host_;
  int port_;
  std::string model_id_;
};
}  // namespace commands