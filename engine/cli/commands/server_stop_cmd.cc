#include "server_stop_cmd.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

namespace commands {
ServerStopCmd::ServerStopCmd(std::string host, int port)
    : host_(std::move(host)), port_(port) {}

void ServerStopCmd::Exec() {
  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host_ + ":" + std::to_string(port_),
      /* .pathParams = */ {"processManager", "destroy"},
      /* .queries = */ {},
  };

  auto res = curl_utils::SimpleDeleteJson(url.ToFullPath());
  if (res.has_error()) {
    CLI_LOG_ERROR("Failed to stop server: " << res.error());
    return;
  }

  CLI_LOG("Server stopped!");
}
};  // namespace commands
