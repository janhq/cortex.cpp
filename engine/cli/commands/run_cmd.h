#pragma once

#include <string>
#include "services/engine_service.h"
#include "services/model_service.h"

namespace commands {

std::optional<std::string> SelectLocalModel(ModelService& model_service,
                                            const std::string& model_handle);

class RunCmd {
 public:
  explicit RunCmd(std::string host, int port, std::string model_handle,
                  std::shared_ptr<DownloadService> download_service)
      : host_{std::move(host)},
        port_{port},
        model_handle_{std::move(model_handle)},
        engine_service_{EngineService(download_service)},
        model_service_{ModelService(download_service)} {};

  void Exec(bool chat_flag);

 private:
  std::string host_;
  int port_;
  std::string model_handle_;

  ModelService model_service_;
  EngineService engine_service_;
};
}  // namespace commands
