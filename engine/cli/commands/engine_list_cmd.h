#pragma once

#include <string>
#include "services/engine_service.h"

namespace commands {
class EngineListCmd {
 public:
  explicit EngineListCmd(std::shared_ptr<EngineService> engine_service)
      : engine_service_{engine_service} {}

  bool Exec(const std::string& host, int port);

 private:
  std::shared_ptr<EngineService> engine_service_;
};

}  // namespace commands
