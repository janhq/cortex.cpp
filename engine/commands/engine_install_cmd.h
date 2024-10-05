#pragma once

#include <string>
#include "services/engine_service.h"

namespace commands {

class EngineInstallCmd {
 public:
  explicit EngineInstallCmd()
      : engine_service_{EngineService(std::make_shared<DownloadService>())} {};

  void Exec(const std::string& engine, const std::string& version = "latest",
            const std::string& src = "");

 private:
  EngineService engine_service_;
};
}  // namespace commands
