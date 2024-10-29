#pragma once

#include <string>
#include "services/engine_service.h"

namespace commands {

class EngineInstallCmd {
 public:
  explicit EngineInstallCmd(std::shared_ptr<DownloadService> download_service, const std::string& host, int port)
      : engine_service_{EngineService(download_service)}, host_(host), port_(port) {};

  bool Exec(const std::string& engine, const std::string& version = "latest",
            const std::string& src = "");

 private:
  EngineService engine_service_;
  std::string host_;
  int port_;
};
}  // namespace commands
