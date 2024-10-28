#include "engine_uninstall_cmd.h"
#include "httplib.h"
#include "server_start_cmd.h"
#include "utils/logging_utils.h"

namespace commands {

void EngineUninstallCmd::Exec(const std::string& host, int port,
                              const std::string& engine) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }

  // Call API to delete engine
  httplib::Client cli(host + ":" + std::to_string(port));
  auto res = cli.Delete("/v1/engines/" + engine);
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CLI_LOG("Engine " + engine + " uninstalled successfully!");
    } else {
      CTL_ERR("Engine failed to uninstall with status code: " << res->status);
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
  }
}
};  // namespace commands