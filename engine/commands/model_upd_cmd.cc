#include "model_upd_cmd.h"
#include "httplib.h"
#include "json/json.h"
#include "server_start_cmd.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {

ModelUpdCmd::ModelUpdCmd(std::string model_handle)
    : model_handle_(std::move(model_handle)) {}

void ModelUpdCmd::Exec(
    const std::string& host, int port,
    const std::unordered_map<std::string, std::string>& options) {
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
  for (const auto& [key, value] : options) {
    if (!value.empty()) {
      json_data[key] = value;
      CLI_LOG("Updated " << key << " to: " << value);
    }
  }
  auto data_str = json_data.toStyledString();
  auto res = cli.Patch("/v1/models/" + model_handle_, httplib::Headers(),
                       data_str.data(), data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CLI_LOG("Successfully updated model ID '" + model_handle_ + "'!");
      return;
    } else {
      CTL_ERR("Model failed to update with status code: " << res->status);
      return;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return;
  }
}
}  // namespace commands