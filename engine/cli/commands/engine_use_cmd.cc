#include "engine_use_cmd.h"
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

namespace commands {
cpp::result<void, std::string> EngineUseCmd::Exec(const std::string& host,
                                                  int port,
                                                  const std::string& engine) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return cpp::fail("Failed to start server");
    }
  }

  auto get_installed_url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "engines", engine},
      /* .queries =  */ {},
  };
  auto installed_variants_results =
      curl_utils::SimpleGetJson(get_installed_url.ToFullPath());
  if (installed_variants_results.has_error()) {
    CTL_ERR(installed_variants_results.error());
    return cpp::fail("Failed to get installed variants");
  }
  if (installed_variants_results.value().size() == 0) {
    return cpp::fail("No installed variants found");
  }

  std::map<std::string, std::vector<std::string>> variant_map;
  for (const auto& variant : installed_variants_results.value()) {
    auto variant_name = variant["name"].asString();
    if (variant_map.find(variant_name) == variant_map.end()) {
      variant_map[variant_name] = {variant["version"].asString()};
    } else {
      variant_map[variant_name].push_back(variant["version"].asString());
    }
  }

  std::vector<std::string> variant_selections;
  for (const auto& [key, value] : variant_map) {
    variant_selections.push_back(key);
  }

  auto selected_variant =
      cli_selection_utils::PrintSelection(variant_selections);
  if (!selected_variant.has_value()) {
    CTL_ERR("Invalid variant selection");
    return cpp::fail("Invalid variant selection");
  }

  std::optional<std::string> selected_version = std::nullopt;
  if (variant_map[selected_variant.value()].size() == 1) {
    selected_version = variant_map[selected_variant.value()][0];
  } else {
    selected_version = cli_selection_utils::PrintSelection(
        variant_map[selected_variant.value()]);
  }
  if (!selected_version.has_value()) {
    CTL_ERR("Invalid version selection");
    return cpp::fail("Invalid version selection");
  }

  Json::Value body;
  body["variant"] = selected_variant.value();
  body["version"] = selected_version.value();
  auto set_default_engine_variant = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "engines", engine, "default"},
      /* .queries = */ {},
  };

  auto response = curl_utils::SimplePostJson(
      set_default_engine_variant.ToFullPath(), body.toStyledString());
  if (response.has_error()) {
    CTL_ERR(response.error());
    return cpp::fail("Failed to set default engine variant");
  }

  CLI_LOG("Engine " << engine << " updated successfully!");
  return {};
}
};  // namespace commands
