#pragma once

#include "services/engine_service.h"

namespace commands {
class EngineGetCmd {
 public:
  explicit EngineGetCmd(std::shared_ptr<DownloadService> download_service)
      : engine_service_{EngineService(download_service)} {};

  void Exec(const std::string& engineName) const;

 private:
  EngineService engine_service_;
};
}  // namespace commands
