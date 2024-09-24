#include "model_start_cmd.h"
#include "cortex_upd_cmd.h"
#include "httplib.h"
#include "model_status_cmd.h"
#include "nlohmann/json.hpp"
#include "server_start_cmd.h"
#include "trantor/utils/Logger.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/modellist_utils.h"

namespace commands {
bool ModelStartCmd::Exec(const std::string& host, int port,
                         const std::string& model_handle) {

  modellist_utils::ModelListUtils modellist_handler;
  config::YamlHandler yaml_handler;
  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
    auto mc = yaml_handler.GetModelConfig();
    return Exec(host, port, mc);
  } catch (const std::exception& e) {
    CLI_LOG("Fail to start model information with ID '" + model_handle +
            "': " + e.what());
    return false;
  }
}

bool ModelStartCmd::Exec(const std::string& host, int port,
                         const config::ModelConfig& mc) {
  // Check if server is started
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Server is not started yet, please run `"
            << commands::GetCortexBinary() << " start` to start server!");
    return false;
  }

  // Only check for llamacpp for now
  if ((mc.engine.find("llamacpp") != std::string::npos) &&
      commands::ModelStatusCmd().IsLoaded(host, port, mc)) {
    CLI_LOG("Model has already been started!");
    return true;
  }

  httplib::Client cli(host + ":" + std::to_string(port));

  nlohmann::json json_data;
  if (mc.files.size() > 0) {
    // TODO(sang) support multiple files
    json_data["model_path"] = mc.files[0];
  } else {
    LOG_WARN << "model_path is empty";
    return false;
  }
  json_data["model"] = mc.name;
  json_data["system_prompt"] = mc.system_template;
  json_data["user_prompt"] = mc.user_template;
  json_data["ai_prompt"] = mc.ai_template;
  json_data["ctx_len"] = mc.ctx_len;
  json_data["stop"] = mc.stop;
  json_data["engine"] = mc.engine;

  auto data_str = json_data.dump();
  cli.set_read_timeout(std::chrono::seconds(60));
  auto res = cli.Post("/inferences/server/loadmodel", httplib::Headers(),
                      data_str.data(), data_str.size(), "application/json");
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
  return false;
}

};  // namespace commands
