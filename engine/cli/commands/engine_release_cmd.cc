#include "engine_release_cmd.h"
#include <vector>
#include "commands/server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/curl_utils.h"
#include "utils/github_release_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

namespace commands {

cpp::result<void, std::string> EngineReleaseCmd::Exec(
    const std::string& host, int port, const std::string& engine_name) {

  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return cpp::fail("Failed to start server");
    }
  }

  auto url_obj = url_parser::Url{
      .protocol = "http",
      .host = host + ":" + std::to_string(port),
      .pathParams = {"v1", "engines", engine_name, "releases"},
  };

  auto res = curl_utils::SimpleGetJson(url_obj.ToFullPath());
  if (res.has_error()) {
    CLI_LOG("url: " + url_obj.ToFullPath());
    return cpp::fail("Failed to get engine release: " + engine_name +
                     ", error: " + res.error());
  }
  if (res.value().size() == 0) {
    return cpp::fail("No release found for engine: " + engine_name);
  }

  std::vector<std::string> selections;
  for (const auto& release : res.value()) {
    selections.push_back(release["tag_name"].asString());
  }

  auto selection =
      cli_selection_utils::PrintSelection(selections, "Available versions:");
  if (!selection.has_value()) {
    return cpp::fail("Invalid selection!");  // TODO: return latest version
  }

  CLI_LOG("Selected " + engine_name + " version " + selection.value());
  Json::Value selected_release;
  for (const auto& release : res.value()) {
    if (release["tag_name"].asString() == selection.value()) {
      selected_release = release;
      break;
    }
  }

  std::vector<std::string> variant_selections;
  for (const auto& variant : selected_release["assets"]) {
    variant_selections.push_back(variant["name"].asString());
  }

  auto variant_selection = cli_selection_utils::PrintSelection(
      variant_selections, "Available variant:");
  if (!variant_selection.has_value()) {
    return cpp::fail("Invalid variant selection!");
  }

  CLI_LOG("Selected " + variant_selection.value());
  github_release_utils::GitHubAsset selected_asset;
  for (const auto& asset : selected_release["assets"]) {
    if (asset["name"] == variant_selection) {
      auto version = string_utils::RemoveSubstring(selection.value(), "v");
      selected_asset =
          github_release_utils::GitHubAsset::FromJson(asset, version);
      break;
    }
  }

  // proceed to download

  return {};
}
};  // namespace commands
