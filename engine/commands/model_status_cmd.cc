#include "model_status_cmd.h"
#include "utils/logging_utils.h"

namespace commands {
bool ModelStatusCmd::IsLoaded(const std::string& host, int port,
                              const std::string& model_handle) {
  auto res = model_service_.GetModelStatus(host, port, model_handle);

  if (res.has_error()) {
    CTL_ERR("Error: " + res.error());
    return false;
  }
  return true;
}
}  // namespace commands
