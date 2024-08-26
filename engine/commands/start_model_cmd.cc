#include "start_model_cmd.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "trantor/utils/Logger.h"

namespace commands {
StartModelCmd::StartModelCmd(std::string host, int port,
                             const config::ModelConfig& mc)
    : host_(std::move(host)), port_(port), mc_(mc) {}

void StartModelCmd::Exec() {
  httplib::Client cli(host_ + ":" + std::to_string(port_));
  nlohmann::json json_data;
  if (mc_.files.size() > 0) {
    // TODO(sang) support multiple files
    json_data["model_path"] = mc_.files[0];
  } else {
    LOG_WARN << "model_path is empty";
    return;
  }
  json_data["model"] = mc_.name;
  json_data["system_prompt"] = mc_.system_template;
  json_data["user_prompt"] = mc_.user_template;
  json_data["ai_prompt"] = mc_.ai_template;
  json_data["ctx_len"] = mc_.ctx_len;
  json_data["stop"] = mc_.stop;
  json_data["engine"] = mc_.engine;

  auto data_str = json_data.dump();

  auto res = cli.Post("/inferences/server/loadmodel", httplib::Headers(),
                      data_str.data(), data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      LOG_INFO << res->body;
    }
  } else {
    auto err = res.error();
    LOG_WARN << "HTTP error: " << httplib::to_string(err);
  }
}

};  // namespace commands