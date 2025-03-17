#include "model_status_cmd.h"
#include "server_start_cmd.h"
#include "utils/curl_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

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
  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "models", "status", model_handle},
			/* .queries= */ {},
  };

  auto res = curl_utils::SimpleGetJson(url.ToFullPath());
  if (res.has_error()) {
    auto root = json_helper::ParseJsonString(res.error());
    CTL_WRN(root["message"].asString());
    return false;
  }

  CTL_INF(res.value().toStyledString());
  return true;
}
}  // namespace commands
