#include "model_import_cmd.h"
#include <json/value.h>
#include "server_start_cmd.h"
#include "utils/curl_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

namespace commands {

void ModelImportCmd::Exec(const std::string& host, int port,
                          const std::string& model_handle,
                          const std::string& model_path) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }

  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "models", "import"},
      /* .queries = */ {},
  };

  Json::Value json_data;
  json_data["model"] = model_handle;
  json_data["modelPath"] = model_path;
  auto data_str = json_data.toStyledString();

  auto res = curl_utils::SimplePostJson(url.ToFullPath(), data_str);
  if (res.has_error()) {
    auto root = json_helper::ParseJsonString(res.error());
    CLI_LOG(root["message"].asString());
    return;
  }

  CLI_LOG("Successfully import model from  '" + model_path + "' for modelID '" +
          model_handle + "'.");
}
}  // namespace commands
