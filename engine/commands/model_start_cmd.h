#pragma once
#include <string>

namespace commands {

class ModelStartCmd {
 public:
  bool Exec(const std::string& host, int port, const std::string& model_handle);

};
}  // namespace commands
