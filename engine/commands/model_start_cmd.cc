#include "model_start_cmd.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "trantor/utils/Logger.h"
#include "utils/logging_utils.h"

namespace commands {
ModelStartCmd::ModelStartCmd(std::string host, int port,
                             const config::ModelConfig& mc)
    : host_(std::move(host)), port_(port), mc_(mc) {}

bool ModelStartCmd::Exec() {
  httplib::Client cli(host_ + ":" + std::to_string(port_));
  nlohmann::json json_data;
  if (mc_.files.size() > 0) {
    // TODO(sang) support multiple files
    json_data["model_path"] = mc_.files[0];
  } else {
    LOG_WARN << "model_path is empty";
    return false;
  }
  json_data["model"] = mc_.name;
  json_data["system_prompt"] = mc_.system_template;
  json_data["user_prompt"] = mc_.user_template;
  json_data["ai_prompt"] = mc_.ai_template;
  json_data["ctx_len"] = mc_.ctx_len;
  json_data["stop"] = mc_.stop;
  json_data["engine"] = mc_.engine;

  auto data_str = json_data.dump();
  cli.set_read_timeout(std::chrono::seconds(60));
  auto res = cli.Post("/inferences/server/loadmodel", httplib::Headers(),
                      data_str.data(), data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CLI_LOG("Model loaded!");
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return false;
  }
  return true;
}

};  // namespace commands
