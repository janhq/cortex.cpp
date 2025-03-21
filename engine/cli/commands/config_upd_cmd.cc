#include "config_upd_cmd.h"
#include "commands/server_start_cmd.h"
#include "common/api_server_configuration.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"
#include "utils/url_parser.h"

namespace {
inline Json::Value NormalizeJson(
    const std::unordered_map<std::string, std::string> options) {
  Json::Value root;
  for (const auto& [key, value] : options) {
    if (CONFIGURATIONS.find(key) == CONFIGURATIONS.end()) {
      continue;
    }
    auto config = CONFIGURATIONS.at(key);

    if (config.accept_value == "[on|off]") {
      if (string_utils::EqualsIgnoreCase("on", value)) {
        root[key] = true;
      } else if (string_utils::EqualsIgnoreCase("off", value)) {
        root[key] = false;
      }
    } else if (config.accept_value == "comma separated") {
      auto origins = string_utils::SplitBy(value, ",");
      Json::Value origin_array(Json::arrayValue);
      for (const auto& origin : origins) {
        origin_array.append(origin);
      }
      root[key] = origin_array;
    } else if (config.accept_value == "string") {
      root[key] = value;
    } else {
      CTL_ERR("Not support configuration type: " << config.accept_value
                                                 << " for config key: " << key);
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

  auto non_null_opts = std::unordered_map<std::string, std::string>();
  for (const auto& [key, value] : options) {
    if (value.empty() && !CONFIGURATIONS.at(key).allow_empty) {
      continue;
    }
    non_null_opts[key] = value;
  }

  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "configs"},
      /* .queries = */ {},
  };

  auto json = NormalizeJson(non_null_opts);
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
