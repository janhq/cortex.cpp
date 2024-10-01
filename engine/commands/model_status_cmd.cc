#include "model_status_cmd.h"
#include "config/yaml_config.h"
#include "database/models.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "utils/logging_utils.h"
#include "services/model_service.h"

namespace commands {
bool ModelStatusCmd::IsLoaded(const std::string& host, int port,
                              const std::string& model_handle) {
  ModelService ms;
  auto res = ms.GetModelStatus(host, port, model_handle);

  if (res.has_error()) {
    // CLI_LOG("Error: " + res.error());
    return false;
  }
  return true;
}
}  // namespace commands