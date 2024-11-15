#include "model_stop_cmd.h"
#include "httplib.h"
#include "utils/logging_utils.h"

namespace commands {

void ModelStopCmd::Exec(const std::string& host, int port,
                        const std::string& model_handle) {
  // Call API to stop model
  httplib::Client cli(host + ":" + std::to_string(port));
  Json::Value json_data;
  json_data["model"] = model_handle;
  auto data_str = json_data.toStyledString();
  auto res = cli.Post("/v1/models/stop", httplib::Headers(), data_str.data(),
                      data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CLI_LOG("Model unloaded!");
    } else {
      auto root = json_helper::ParseJsonString(res->body);
      CLI_LOG(root["message"].asString());
      return;
    }
  } else {
    auto err = res.error();
    CLI_LOG("HTTP error: " << httplib::to_string(err));
  }
}

};  // namespace commands
