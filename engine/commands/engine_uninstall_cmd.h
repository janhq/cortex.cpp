#pragma once

#include <memory>
#include <string>
#include "services/engine_service.h"

namespace commands {
class EngineUninstallCmd {
 public:
  explicit EngineUninstallCmd()
      : engine_service_{EngineService(std::make_shared<DownloadService>())} {};

  void Exec(const std::string& engine);

 private:
  EngineService engine_service_;
};
}  // namespace commands
