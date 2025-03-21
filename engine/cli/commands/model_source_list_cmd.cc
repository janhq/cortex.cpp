#include "model_source_list_cmd.h"
#include <json/reader.h>
#include <json/value.h>
#include <iostream>
#include <vector>
#include "server_start_cmd.h"
#include "utils/curl_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"
#include "utils/url_parser.h"
// clang-format off
#include <tabulate/table.hpp>
// clang-format on

namespace commands {

bool ModelSourceListCmd::Exec(const std::string& host, int port) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return false;
    }
  }

  tabulate::Table table;
  table.add_row({"#", "Model Source"});

  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "models", "sources"},
      /* .queries = */ {},
  };
  auto result = curl_utils::SimpleGetJson(url.ToFullPath());
  if (result.has_error()) {
    CTL_ERR(result.error());
    return false;
  }
  table.format().font_color(tabulate::Color::green);
  int count = 0;

  if (!result.value()["data"].isNull()) {
    for (auto const& v : result.value()["data"]) {
      auto model_source = v.asString();
      count += 1;
      std::vector<std::string> row = {std::to_string(count), model_source};
      table.add_row({row.begin(), row.end()});
    }
  }

  std::cout << table << std::endl;
  return true;
}
};  // namespace commands
