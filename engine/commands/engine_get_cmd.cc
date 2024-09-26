#include "engine_get_cmd.h"
#include <iostream>
#include <tabulate/table.hpp>
#include "services/engine_service.h"
#include "utils/logging_utils.h"

namespace commands {

void EngineGetCmd::Exec(const std::string& engine_name) const {
  auto engine = engine_service_.GetEngineInfo(engine_name);
  if (engine.has_error()) {
    CLI_LOG(engine.error());
    return;
  }

  auto version = engine->version.value_or("");
  auto variant = engine->variant.value_or("");
  tabulate::Table table;
  table.add_row({"Name", "Supported Formats", "Version", "Variant", "Status"});
  table.add_row(
      {engine->product_name, engine->format, version, variant, engine->status});
  std::cout << table << std::endl;
}
};  // namespace commands
