#include "model_status_cmd.h"
#include "httplib.h"
#include "server_start_cmd.h"
#include "utils/logging_utils.h"

namespace commands {
bool ModelStatusCmd::IsLoaded(const std::string& host, int port,
                              const std::string& model_handle) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return false;
    }
  }

  // Call API to check model status
  httplib::Client cli(host + ":" + std::to_string(port));
  auto res = cli.Get("/v1/models/status/" + model_handle);
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CTL_INF(res->body);
    } else {
      CTL_WRN("Failed to get model status with code: " << res->status);
      return false;
    }
  } else {
    auto err = res.error();
    CTL_WRN("HTTP error: " << httplib::to_string(err));
    return false;
  }
  return true;
}
}  // namespace commands
