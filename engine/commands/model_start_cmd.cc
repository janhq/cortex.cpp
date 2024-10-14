#include "model_start_cmd.h"
#include "httplib.h"
#include "server_start_cmd.h"
#include "utils/logging_utils.h"

namespace commands {
bool ModelStartCmd::Exec(const std::string& host, int port,
                         const std::string& model_handle) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return false;
    }
  }
  // Call API to start model
  httplib::Client cli(host + ":" + std::to_string(port));
  Json::Value json_data;
  json_data["model"] = model_handle;
  auto data_str = json_data.toStyledString();
  auto res = cli.Post("/v1/models/start", httplib::Headers(), data_str.data(),
                      data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CLI_LOG("Model loaded!");
      return true;
    } else {
      CTL_ERR("Model failed to load with status code: " << res->status);
      return false;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return false;
  }
}

};  // namespace commands
