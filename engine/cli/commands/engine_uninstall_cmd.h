#pragma once

#include <string>

namespace commands {
class EngineUninstallCmd {
 public:
  void Exec(const std::string& host, int port, const std::string& engine);
};
}  // namespace commands
