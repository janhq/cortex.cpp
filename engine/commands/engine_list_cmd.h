#pragma once

#include "services/engine_service.h"

namespace commands {
class EngineListCmd {
 public:
  explicit EngineListCmd(std::shared_ptr<DownloadService> download_service)
      : engine_service_{EngineService(download_service)} {};

  bool Exec();

 private:
  EngineService engine_service_;
};

}  // namespace commands
