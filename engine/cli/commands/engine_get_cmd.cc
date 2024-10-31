#include "engine_get_cmd.h"
#include <json/reader.h>
#include <json/value.h>
#include <iostream>
#include "server_start_cmd.h"
#include "services/engine_service.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

// clang-format off
#include <tabulate/table.hpp>
// clang-format on

namespace commands {

void EngineGetCmd::Exec(const std::string& host, int port,
                        const std::string& engine_name) const {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }

  tabulate::Table table;
  table.add_row({"#", "Name", "Version", "Variant", "Status"});

  auto url = url_parser::Url{
      .protocol = "http",
      .host = host + ":" + std::to_string(port),
      .pathParams = {"v1", "engines", engine_name},
  };
  auto result = curl_utils::SimpleGetJson(url.ToFullPath());
  if (result.has_error()) {
    CTL_ERR(result.error());
    return;
  }

  std::vector<EngineVariantResponse> output;
  auto installed_variants = result.value();
  for (const auto& variant : installed_variants) {
    output.push_back(EngineVariantResponse{
        .name = variant["name"].asString(),
        .version = variant["version"].asString(),
        .engine = engine_name,
    });
  }

  int count = 0;
  for (auto const& v : output) {
    count += 1;
    table.add_row(
        {std::to_string(count), v.engine, v.version, v.name, "Installed"});
  }

  std::cout << table << std::endl;
}
};  // namespace commands
