#include "model_stop_cmd.h"
#include "config/yaml_config.h"
#include "database/models.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "services/model_service.h"

namespace commands {

void ModelStopCmd::Exec(const std::string& host, int port,
                        const std::string& model_handle) {
  ModelService ms;
  auto res = ms.StopModel(host, port, model_handle);

  if (res.has_error()) {
    CLI_LOG("Error: " + res.error());
    return;
  }
  CLI_LOG("Model unloaded!");
}

};  // namespace commands
