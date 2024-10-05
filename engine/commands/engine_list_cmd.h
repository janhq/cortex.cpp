#pragma once

#include "services/engine_service.h"

namespace commands {
class EngineListCmd {
 public:
  explicit EngineListCmd()
      : engine_service_{EngineService(std::make_shared<DownloadService>())} {};

  bool Exec();

 private:
  EngineService engine_service_;
};

}  // namespace commands
