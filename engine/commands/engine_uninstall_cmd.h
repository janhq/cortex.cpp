#pragma once

#include <string>

namespace commands {
class EngineUninstallCmd {
 public:
  EngineUninstallCmd(std::string engine);

  void Exec() const;

 private:
  std::string engine_;
};
}  // namespace commands
