#include "config_get_cmd.h"
#include "commands/server_start_cmd.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"
// clang-format off
#include <tabulate/table.hpp>
// clang-format on

void commands::ConfigGetCmd::Exec(const std::string& host, int port) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }
  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "configs"},
      /* .queries = */ {},
  };

  auto get_config_result = curl_utils::SimpleGetJson(url.ToFullPath());
  if (get_config_result.has_error()) {
    CLI_LOG_ERROR(
        "Failed to get configurations: " << get_config_result.error());
    return;
  }

  auto json_value = get_config_result.value();
  tabulate::Table table;
  table.add_row({"Config name", "Value"});

  for (const auto& key : json_value.getMemberNames()) {
    if (json_value[key].isArray()) {
      for (const auto& value : json_value[key]) {
        table.add_row({key, value.asString()});
      }
    } else {
      table.add_row({key, json_value[key].asString()});
    }
  }

  std::cout << table << std::endl;
  return;
}
