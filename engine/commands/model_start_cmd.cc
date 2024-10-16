#include "model_start_cmd.h"
#include "utils/logging_utils.h"

namespace commands {
bool ModelStartCmd::Exec(const std::string& host, int port,
                         const std::string& model_handle) {
  auto res = model_service_.StartModel(host, port, model_handle);

  if (res.has_error()) {
    CLI_LOG("Error: " + res.error());
    return false;
  }

  CLI_LOG("Model loaded!");
  return true;
}

};  // namespace commands
