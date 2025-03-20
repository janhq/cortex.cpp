#include "model_stop_cmd.h"
#include <json/value.h>
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

namespace commands {

void ModelStopCmd::Exec(const std::string& host, int port,
                        const std::string& model_handle) {
  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "models", "stop"},
      /* .queries = */ {},
  };

  Json::Value json_data;
  json_data["model"] = model_handle;
  auto data_str = json_data.toStyledString();
  auto res = curl_utils::SimplePostJson(url.ToFullPath(), data_str);

  if (res.has_error()) {
    CLI_LOG_ERROR("Failed to stop model: " << res.error());
    return;
  }

  CLI_LOG("Model stopped!");
}
};  // namespace commands
