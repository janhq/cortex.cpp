#include "engine_get_cmd.h"
#include <iostream>
#include <tabulate/table.hpp>
#include "services/engine_service.h"
#include "utils/logging_utils.h"

namespace commands {

void EngineGetCmd::Exec(const std::string& engine_name) const {
  auto engine = engine_service_.GetEngineInfo(engine_name);
  if (engine == std::nullopt) {
    CLI_LOG("Engine " + engine_name + " is not supported!");
    return;
  }

  tabulate::Table table;
  table.add_row({"Name", "Supported Formats", "Version", "Status"});
  table.add_row(
      {engine->product_name, engine->format, engine->version, engine->status});
  std::cout << table << std::endl;
}
};  // namespace commands
