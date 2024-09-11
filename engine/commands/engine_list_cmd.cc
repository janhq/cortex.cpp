#include "engine_list_cmd.h"
#include <tabulate/table.hpp>
#include "services/engine_service.h"

namespace commands {

bool EngineListCmd::Exec() {
  auto engine_service = EngineService();
  auto status_list = engine_service.GetEngineInfoList();

  tabulate::Table table;
  table.add_row({"#", "Name", "Supported Formats", "Version", "Status"});
  for (int i = 0; i < status_list.size(); i++) {
    auto status = status_list[i];
    std::string index = std::to_string(i + 1);
    table.add_row({index, status.product_name, status.format, status.version,
                   status.status});
  }

  std::cout << table << std::endl;
  return true;
}
};  // namespace commands
