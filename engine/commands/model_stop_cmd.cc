#include "model_stop_cmd.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "trantor/utils/Logger.h"
#include "utils/logging_utils.h"

namespace commands {
ModelStopCmd::ModelStopCmd(std::string host, int port,
                           const config::ModelConfig& mc)
    : host_(std::move(host)), port_(port), mc_(mc) {}

void ModelStopCmd::Exec() {
  httplib::Client cli(host_ + ":" + std::to_string(port_));
  nlohmann::json json_data;
  json_data["model"] = mc_.name;
  json_data["engine"] = mc_.engine;

  auto data_str = json_data.dump();

  auto res = cli.Post("/inferences/server/unloadmodel", httplib::Headers(),
                      data_str.data(), data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      // LOG_INFO << res->body;
      CLI_LOG("Model unloaded!");
    }
  } else {
    auto err = res.error();
    CTLOG_ERROR("HTTP error: " << httplib::to_string(err));
  }
}

};  // namespace commands