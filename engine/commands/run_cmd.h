#pragma once
#include <string>
#include "services/engine_service.h"
#include "services/model_service.h"

namespace commands {
class RunCmd {
 public:
  explicit RunCmd(std::string host, int port, std::string model_id)
      : host_{std::move(host)},
        port_{port},
        model_id_{std::move(model_id)},
        model_service_{ModelService()} {};

  void Exec();

 private:
  std::string host_;
  int port_;
  std::string model_id_;

  ModelService model_service_;
  EngineService engine_service_;
};
}  // namespace commands
