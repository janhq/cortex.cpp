#include "engine_get_cmd.h"
#include <iostream>

#include "httplib.h"
#include "json/json.h"
#include "server_start_cmd.h"
#include "utils/logging_utils.h"

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
  table.add_row({"Name", "Supported Formats", "Version", "Variant", "Status"});
  httplib::Client cli(host + ":" + std::to_string(port));
  auto res = cli.Get("/v1/engines/" + engine_name);
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      // CLI_LOG(res->body);
      Json::Value v;
      Json::Reader reader;
      reader.parse(res->body, v);

      table.add_row({v["name"].asString(), v["format"].asString(),
                     v["version"].asString(), v["variant"].asString(),
                     v["status"].asString()});

    } else {
      CLI_LOG_ERROR("Failed to get engine list with status code: " << res->status);
      return;
    }
  } else {
    auto err = res.error();
    CLI_LOG_ERROR("HTTP error: " << httplib::to_string(err));
    return;
  }

  std::cout << table << std::endl;
}
};  // namespace commands
