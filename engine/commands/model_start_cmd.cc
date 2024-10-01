#include "model_start_cmd.h"
#include "cortex_upd_cmd.h"
#include "database/models.h"
#include "httplib.h"
#include "model_status_cmd.h"
#include "nlohmann/json.hpp"
#include "server_start_cmd.h"
#include "services/model_service.h"
#include "trantor/utils/Logger.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {
bool ModelStartCmd::Exec(const std::string& host, int port,
                         const std::string& model_handle) {
  ModelService ms;
  auto res = ms.StartModel(host, port, model_handle);

  if (res.has_error()) {
    CLI_LOG("Error: " + res.error());
    return false;
  }
  CLI_LOG("Model loaded!");
  return true;
}

};  // namespace commands
