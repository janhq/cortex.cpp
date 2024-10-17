#include "engine_list_cmd.h"
#include "httplib.h"
#include "json/json.h"
#include "server_start_cmd.h"
#include "utils/logging_utils.h"
// clang-format off
#include <tabulate/table.hpp>
// clang-format on

namespace commands {

bool EngineListCmd::Exec(const std::string& host, int port) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return false;
    }
  }

  tabulate::Table table;
  table.add_row(
      {"#", "Name", "Supported Formats", "Version", "Variant", "Status"});

  httplib::Client cli(host + ":" + std::to_string(port));
  auto res = cli.Get("/v1/engines");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      int count = 0;
      // CLI_LOG(res->body);
      Json::Value body;
      Json::Reader reader;
      reader.parse(res->body, body);
      if (!body["data"].isNull()) {
        for (auto const& v : body["data"]) {
          count += 1;
          table.add_row({std::to_string(count), v["name"].asString(),
                         v["format"].asString(), v["version"].asString(),
                         v["variant"].asString(), v["status"].asString()});
        }
      }
    } else {
      CLI_LOG_ERROR("Failed to get engine list with status code: " << res->status);
      return false;
    }
  } else {
    auto err = res.error();
    CLI_LOG_ERROR("HTTP error: " << httplib::to_string(err));
    return false;
  }

  std::cout << table << std::endl;
  return true;
}
};  // namespace commands
