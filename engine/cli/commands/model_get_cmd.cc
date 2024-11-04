#include "model_get_cmd.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>
#include "config/yaml_config.h"
#include "database/models.h"
#include "httplib.h"
#include "server_start_cmd.h"
#include "utils/file_manager_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"

namespace commands {

void ModelGetCmd::Exec(const std::string& host, int port,
                       const std::string& model_handle) {
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
  auto res = cli.Get("/v1/models/" + model_handle);
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CLI_LOG(res->body);
    } else {
      auto root = json_helper::ParseJsonString(res->body);
      CLI_LOG(root["message"].asString());
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
  }
}

}  // namespace commands