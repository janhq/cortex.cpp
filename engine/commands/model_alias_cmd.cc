#include "model_alias_cmd.h"
#include "database/models.h"
#include "httplib.h"
#include "server_start_cmd.h"
#include "json/json.h"

namespace commands {

void ModelAliasCmd::Exec(const std::string& host, int port,
                         const std::string& model_handle,
                         const std::string& model_alias) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }

  // Call API to delete model
  httplib::Client cli(host + ":" + std::to_string(port));
  Json::Value json_data;
  json_data["model"] = model_handle;
  json_data["modelAlias"] = model_alias;
  auto data_str = json_data.toStyledString();
  auto res = cli.Post("/v1/models/alias", httplib::Headers(), data_str.data(),
                      data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CLI_LOG("Successfully set model alias '" + model_alias +
                "' for modeID '" + model_handle + "'.");
    } else {
      CTL_ERR("Model failed to set alias with status code: " << res->status);
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
  }
}

}  // namespace commands