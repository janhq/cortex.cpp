#include "model_list_cmd.h"
#include <iostream>

#include <vector>
#include "httplib.h"
#include "json/json.h"
#include "server_start_cmd.h"
#include "utils/logging_utils.h"
// clang-format off
#include <tabulate/table.hpp>
// clang-format on
namespace commands {

void ModelListCmd::Exec(const std::string& host, int port) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }

  tabulate::Table table;

  table.add_row({"(Index)", "ID", "model alias", "engine", "version"});
  table.format().font_color(tabulate::Color::green);
  int count = 0;
  // Iterate through directory

  httplib::Client cli(host + ":" + std::to_string(port));
  auto res = cli.Get("/v1/models");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      // CLI_LOG(res->body);
      Json::Value body;
      Json::Reader reader;
      reader.parse(res->body, body);
      if (!body["data"].isNull()) {
        for (auto const& v : body["data"]) {
          count += 1;
          table.add_row({std::to_string(count), v["model"].asString(),
                         v["model_alias"].asString(), v["engine"].asString(),
                         v["version"].asString()});
        }
      }
    } else {
      CTL_ERR("Failed to get model list with status code: " << res->status);
      return;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return;
  }

  for (int i = 0; i < 5; i++) {
    table[0][i]
        .format()
        .font_color(tabulate::Color::white)  // Set font color
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center);
  }
  for (int i = 1; i <= count; i++) {
    table[i][0]  //index value
        .format()
        .font_color(tabulate::Color::white)  // Set font color
        .font_align(tabulate::FontAlign::center);
    table[i][4]  //version value
        .format()
        .font_align(tabulate::FontAlign::center);
  }
  std::cout << table << std::endl;
}
}

;  // namespace commands
