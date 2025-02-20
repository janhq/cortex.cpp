#pragma once

#include <string>
#include <unordered_map>
#include "services/database_service.h"
#include "services/engine_service.h"

namespace commands {

std::optional<std::string> SelectLocalModel(std::string host, int port,
                                            const std::string& model_handle,
                                            DatabaseService& db_service);

class RunCmd {
 public:
  explicit RunCmd(std::string host, int port, std::string model_handle,
                  std::shared_ptr<DatabaseService> db_service,
                  std::shared_ptr<EngineService> engine_service)
      : host_{std::move(host)},
        port_{port},
        model_handle_{std::move(model_handle)},
        db_service_(db_service),
        engine_service_{engine_service} {};

  void Exec(bool chat_flag,
            const std::unordered_map<std::string, std::string>& options);

 private:
  std::string host_;
  int port_;
  std::string model_handle_;
  std::shared_ptr<DatabaseService> db_service_;
  std::shared_ptr<EngineService> engine_service_;
};
}  // namespace commands
