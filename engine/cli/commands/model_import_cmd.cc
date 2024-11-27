#include "model_import_cmd.h"
#include <json/value.h>
#include "httplib.h"
#include "server_start_cmd.h"
#include "utils/logging_utils.h"

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

  httplib::Client cli(host + ":" + std::to_string(port));
  Json::Value json_data;
  json_data["model"] = model_handle;
  json_data["modelPath"] = model_path;
  auto data_str = json_data.toStyledString();
  auto res = cli.Post("/v1/models/import", httplib::Headers(), data_str.data(),
                      data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CLI_LOG("Successfully import model from  '" + model_path +
              "' for modeID '" + model_handle + "'.");
    } else {
      CTL_ERR("Model failed to import model with status code: " << res->status);
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
  }
}
}  // namespace commands
