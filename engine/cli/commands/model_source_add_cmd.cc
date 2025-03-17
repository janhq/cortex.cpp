#include "model_source_add_cmd.h"
#include "server_start_cmd.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
namespace commands {
bool ModelSourceAddCmd::Exec(const std::string& host, int port, const std::string& model_source) {
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
      /* .pathParams = */ {"v1", "models", "sources"},
      /* .queries = */ {},
  };

  Json::Value json_data;
  json_data["source"] = model_source;

  auto data_str = json_data.toStyledString();
  auto res = curl_utils::SimplePostJson(url.ToFullPath(), data_str);
  if (res.has_error()) {
    auto root = json_helper::ParseJsonString(res.error());
    CLI_LOG(root["message"].asString());
    return false;
  }

  CLI_LOG("Added model source: " << model_source);
  return true;
}


};  // namespace commands
