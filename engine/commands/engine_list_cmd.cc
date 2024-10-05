#include "engine_list_cmd.h"
#include <tabulate/table.hpp>

namespace commands {

bool EngineListCmd::Exec() {
  auto status_list = engine_service_.GetEngineInfoList();

  tabulate::Table table;
  table.add_row(
      {"#", "Name", "Supported Formats", "Version", "Variant", "Status"});
  for (int i = 0; i < status_list.size(); i++) {
    auto engine_status = status_list[i];
    std::string index = std::to_string(i + 1);
    auto variant = engine_status.variant.value_or("");
    auto version = engine_status.version.value_or("");
    table.add_row({index, engine_status.product_name, engine_status.format,
                   version, variant, engine_status.status});
  }

  std::cout << table << std::endl;
  return true;
}
};  // namespace commands
