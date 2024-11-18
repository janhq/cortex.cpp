#pragma once

#include <string>
namespace commands {

class ModelGetCmd {
 public:
  void Exec(const std::string& host, int port, const std::string& model_handle);
};
}  // namespace commands
