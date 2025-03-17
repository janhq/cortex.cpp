#include "model_start_cmd.h"
#include "cortex_upd_cmd.h"
#include "hardware_activate_cmd.h"
#include "run_cmd.h"
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"

namespace commands {
bool ModelStartCmd::Exec(
    const std::string& host, int port, const std::string& model_handle,
    const std::unordered_map<std::string, std::string>& options,
    bool print_success_log) {
  std::optional<std::string> model_id =
      SelectLocalModel(host, port, model_handle, *db_service_);

  if (!model_id.has_value()) {
    return false;
  }

  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return false;
    }
  }

  //
  bool should_activate_hw = false;
  for (auto const& [k, v] : options) {
    if (k == "gpus" && !v.empty()) {
      should_activate_hw = true;
      break;
    }
  }
  if (should_activate_hw) {
    if (!HardwareActivateCmd().Exec(host, port, options)) {
      return false;
    }
    // wait for server up, max for 3 seconds
    int count = 6;
    while (count--) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      if (commands::IsServerAlive(host, port))
        break;
    }
  }

  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "models", "start"},
      /* .queries = */ {},
  };

  Json::Value json_data;
  json_data["model"] = model_id.value();
  for (auto const& [k, v] : options) {
    UpdateConfig(json_data, k, v);
  }
  auto data_str = json_data.toStyledString();
  auto res = curl_utils::SimplePostJson(url.ToFullPath(), data_str);
  if (res.has_error()) {
    auto root = json_helper::ParseJsonString(res.error());
    CLI_LOG(root["message"].asString());
    return false;
  }

  if (print_success_log) {
    CLI_LOG(model_id.value() << " model started successfully. Use `"
                             << commands::GetCortexBinary() << " run "
                             << *model_id << "` for interactive chat shell");
  }
  if (!res.value()["warning"].isNull()) {
    CLI_LOG(res.value()["warning"].asString());
  }
  return true;
}

bool ModelStartCmd::UpdateConfig(Json::Value& data, const std::string& key,
                                 const std::string& value) {
  if (key == "ctx_len" && !value.empty()) {
    try {
      data["ctx_len"] = std::stoi(value);
    } catch (const std::exception& e) {
      CLI_LOG("Failed to parse numeric value for " << key << ": " << e.what());
    }
  }
  return true;
}
};  // namespace commands
