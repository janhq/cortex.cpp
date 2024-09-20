#include "model_status_cmd.h"
#include "config/yaml_config.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "utils/logging_utils.h"

namespace commands {
bool ModelStatusCmd::IsLoaded(const std::string& host, int port,
                              const config::ModelConfig& mc) {
  httplib::Client cli(host + ":" + std::to_string(port));
  nlohmann::json json_data;
  json_data["model"] = mc.name;
  json_data["engine"] = mc.engine;

  auto data_str = json_data.dump();

  auto res = cli.Post("/inferences/server/modelstatus", httplib::Headers(),
                      data_str.data(), data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      return true;
    }
  } else {
    auto err = res.error();
    CTL_WRN("HTTP error: " << httplib::to_string(err));
    return false;
  }

  return false;
}
}  // namespace commands