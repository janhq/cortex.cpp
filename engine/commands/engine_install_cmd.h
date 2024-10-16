#pragma once

#include <string>
#include "services/engine_service.h"

namespace commands {

class EngineInstallCmd {
 public:
  explicit EngineInstallCmd(std::shared_ptr<DownloadService> download_service)
      : engine_service_{EngineService(download_service)} {};

  void Exec(const std::string& engine, const std::string& version = "latest",
            const std::string& src = "");

 private:
  EngineService engine_service_;
};
}  // namespace commands
