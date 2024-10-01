#pragma once
#include <string>
#include "nlohmann/json.hpp"
#include "services/engine_service.h"
#include "services/model_service.h"

namespace commands {
class RunCmd {
 public:
  explicit RunCmd(std::string host, int port, std::string model_handle)
      : host_{std::move(host)},
        port_{port},
        model_handle_{std::move(model_handle)},
        model_service_{ModelService()} {};

  void Exec(bool chat_flag);

 private:
  std::string host_;
  int port_;
  std::string model_handle_;

  ModelService model_service_;
  EngineService engine_service_;
};
}  // namespace commands
