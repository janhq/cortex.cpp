#pragma once

#include "services/engine_service.h"

namespace commands {
class EngineGetCmd {
 public:
  explicit EngineGetCmd() : engine_service_{EngineService()} {};

  void Exec(const std::string& engineName) const;

 private:
  EngineService engine_service_;
};
}  // namespace commands
