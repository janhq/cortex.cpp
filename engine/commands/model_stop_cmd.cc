#include "model_stop_cmd.h"
#include "utils/logging_utils.h"

namespace commands {

void ModelStopCmd::Exec(const std::string& host, int port,
                        const std::string& model_handle) {
  auto res = model_service_.StopModel(host, port, model_handle);

  if (res.has_error()) {
    CLI_LOG("Error: " + res.error());
    return;
  }
  CLI_LOG("Model unloaded!");
}

};  // namespace commands
