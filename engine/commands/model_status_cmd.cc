#include "model_status_cmd.h"
#include "config/yaml_config.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "utils/logging_utils.h"
#include "database/models.h"

namespace commands {
bool ModelStatusCmd::IsLoaded(const std::string& host, int port,
                              const std::string& model_handle) {
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;
  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    if(model_entry.has_error()) {
      CLI_LOG("Error: " + model_entry.error());
      return false;
    }
    yaml_handler.ModelConfigFromFile(model_entry.value().path_to_model_yaml);
    auto mc = yaml_handler.GetModelConfig();
    return IsLoaded(host, port, mc);
  } catch (const std::exception& e) {
    CLI_LOG("Fail to get model status with ID '" + model_handle +
            "': " + e.what());
    return false;
  }
}

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