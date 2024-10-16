#pragma once

#include <memory>
#include <string>
#include "services/engine_service.h"

namespace commands {
class EngineUninstallCmd {
 public:
  explicit EngineUninstallCmd(std::shared_ptr<DownloadService> download_service)
      : engine_service_{EngineService(download_service)} {};

  void Exec(const std::string& engine);

 private:
  EngineService engine_service_;
};
}  // namespace commands
