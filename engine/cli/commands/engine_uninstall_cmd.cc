#include "engine_uninstall_cmd.h"
#include "server_start_cmd.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

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

  auto url =
      url_parser::Url{/* .protocol = */ "http",
                      /* .host = */ host + ":" + std::to_string(port),
                      /* .pathParams = */ {"v1", "engines", engine, "install"},
                      /* .queries = */ {},
                    };

  auto result = curl_utils::SimpleDeleteJson(url.ToFullPath());
  if (result.has_error()) {
    CTL_ERR(result.error());
    return;
  }

  CLI_LOG("Engine " + engine + " uninstalled successfully!");
}
};  // namespace commands
