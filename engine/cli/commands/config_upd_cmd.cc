#include "config_upd_cmd.h"
#include "commands/server_start_cmd.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"
#include "utils/url_parser.h"

namespace {
const std::vector<std::string> config_keys{"cors", "allowed_origins"};

inline Json::Value NormalizeJson(
    const std::unordered_map<std::string, std::string> options) {
  Json::Value root;
  for (const auto& [key, value] : options) {
    if (std::find(config_keys.begin(), config_keys.end(), key) ==
        config_keys.end()) {
      continue;
    }

    if (key == "cors") {
      if (string_utils::EqualsIgnoreCase("on", value)) {
        root["cors"] = true;
      } else if (string_utils::EqualsIgnoreCase("off", value)) {
        root["cors"] = false;
      }
    } else if (key == "allowed_origins") {
      auto origins = string_utils::SplitBy(value, ",");
      Json::Value origin_array(Json::arrayValue);
      for (const auto& origin : origins) {
        origin_array.append(origin);
      }
      root[key] = origin_array;
    }
  }

  CTL_DBG("Normalized config update request: " << root.toStyledString());

  return root;
}
};  // namespace

void commands::ConfigUpdCmd::Exec(
    const std::string& host, int port,
    const std::unordered_map<std::string, std::string>& options) {
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }

  auto url = url_parser::Url{
      .protocol = "http",
      .host = host + ":" + std::to_string(port),
      .pathParams = {"v1", "configs"},
  };

  auto json = NormalizeJson(options);
  if (json.empty()) {
    CLI_LOG_ERROR("Invalid configuration options provided");
    return;
  }

  auto update_cnf_result =
      curl_utils::SimplePatchJson(url.ToFullPath(), json.toStyledString());
  if (update_cnf_result.has_error()) {
    CLI_LOG_ERROR(update_cnf_result.error());
    return;
  }

  CLI_LOG("Configuration updated successfully!");
}
