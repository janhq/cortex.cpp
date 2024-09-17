#pragma once

#include <string>
#include "services/engine_service.h"

namespace commands {
class EngineUninstallCmd {
 public:
  explicit EngineUninstallCmd() : engine_service_{EngineService()} {};

  void Exec(const std::string& engine);

 private:
  EngineService engine_service_;
};
}  // namespace commands
