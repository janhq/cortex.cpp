#pragma once

#include <string>

namespace commands {

class EngineUpdateCmd {
 public:
  bool Exec(const std::string& host, int port, const std::string& engine);
};
}  // namespace commands
