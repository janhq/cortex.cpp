#include "model_stop_cmd.h"
#include "config/yaml_config.h"
#include "database/models.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {

void ModelStopCmd::Exec(const std::string& host, int port,
                        const std::string& model_handle) {
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;
  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    if (model_entry.has_error()) {
      CLI_LOG("Error: " + model_entry.error());
      return;
    }
    yaml_handler.ModelConfigFromFile(model_entry.value().path_to_model_yaml);
    auto mc = yaml_handler.GetModelConfig();
    httplib::Client cli(host + ":" + std::to_string(port));
    nlohmann::json json_data;
    json_data["model"] = mc.name;
    json_data["engine"] = mc.engine;

    auto data_str = json_data.dump();

    auto res = cli.Post("/inferences/server/unloadmodel", httplib::Headers(),
                        data_str.data(), data_str.size(), "application/json");
    if (res) {
      if (res->status == httplib::StatusCode::OK_200) {
        // LOG_INFO << res->body;
        CLI_LOG("Model unloaded!");
      } else {
        CLI_LOG("Error: could not unload model - " << res->status);
      }
    } else {
      auto err = res.error();
      CTL_ERR("HTTP error: " << httplib::to_string(err));
    }
  } catch (const std::exception& e) {
    CLI_LOG("Fail to stop model information with ID '" + model_handle +
            "': " + e.what());
  }
}

};  // namespace commands
