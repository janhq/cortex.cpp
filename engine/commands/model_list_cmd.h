#pragma once
#include <string>

namespace commands {

class ModelListCmd {
 public:
  void Exec(const std::string& host, int port);
};
}  // namespace commands
