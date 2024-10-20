#include "ps_cmd.h"
#include <httplib.h>
#include <string>
#include <tabulate/table.hpp>
#include "nlohmann/json.hpp"
#include "utils/engine_constants.h"
#include "utils/format_utils.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

namespace commands {

void PsCmd::Exec(const std::string& host, int port) {
  auto host_and_port{host + ":" + std::to_string(port)};
  httplib::Client cli(host_and_port);

  auto res = cli.Get("/inferences/server/models");
  if (!res || res->status != httplib::StatusCode::OK_200) {
    CLI_LOG("No model loaded!");
    return;
  }

  auto body = nlohmann::json::parse(res->body);
  auto data = body["data"];
  std::vector<ModelLoadedStatus> model_status_list;
  try {
    for (const auto& item : data) {
      ModelLoadedStatus model_status;
      // TODO(sang) hardcode for now
      model_status.engine = kLlamaEngine;
      model_status.model = item["id"];
      model_status.ram = item["ram"];
      model_status.start_time = item["start_time"];
      model_status.vram = item["vram"];
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
    CLI_LOG("No model loaded!");
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
