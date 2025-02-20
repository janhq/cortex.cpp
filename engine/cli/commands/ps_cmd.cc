#include "ps_cmd.h"
#include <string>
#include <tabulate/table.hpp>
#include "utils/curl_utils.h"
#include "utils/engine_constants.h"
#include "utils/format_utils.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"
#include "utils/url_parser.h"

namespace commands {

void PsCmd::Exec(const std::string& host, int port) {
  auto url = url_parser::Url{
      .protocol = "http",
      .host = host + ":" + std::to_string(port),
      .pathParams = {"inferences", "server", "models"},
  };
  auto res = curl_utils::SimpleGetJson(url.ToFullPath());
  if (res.has_error()) {
    CLI_LOG("No models loaded!");
    return;
  }

  std::vector<ModelLoadedStatus> model_status_list;
  try {
    for (const auto& item : res.value()["data"]) {
      ModelLoadedStatus model_status;
      // TODO(sang) hardcode for now
      model_status.engine = item["engine"].isNull()
                            ? kLlamaEngine : item["engine"].asString();
      model_status.model = item["id"].asString();
      model_status.ram = item["ram"].asUInt64();
      model_status.start_time = item["start_time"].asUInt64();
      model_status.vram = item["vram"].asUInt64();
      model_status_list.push_back(model_status);
    }
  } catch (const std::exception& e) {
    CLI_LOG("Fail to get list model information: " + std::string(e.what()));
  }

  PrintModelStatusList(model_status_list);
}

void PsCmd::PrintModelStatusList(
    const std::vector<ModelLoadedStatus>& model_status_list) const {
  if (model_status_list.empty()) {
    CLI_LOG("No models loaded!");
    return;
  }

  tabulate::Table table;
  table.add_row({"Model", "Engine", "RAM", "VRAM", "Up time"});
  for (const auto& model_status : model_status_list) {
    table.add_row({
        model_status.model,
        model_status.engine,
        format_utils::BytesToHumanReadable(model_status.ram),
        format_utils::BytesToHumanReadable(model_status.vram),
        string_utils::FormatTimeElapsed(model_status.start_time),
    });
  }
  std::cout << table << std::endl;
}

};  // namespace commands
