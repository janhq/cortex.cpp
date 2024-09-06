#include "engine_list_cmd.h"
#include <tabulate/table.hpp>
#include "services/engine_service.h"

namespace commands {

bool EngineListCmd::Exec() {
  auto engine_service = EngineService();
  auto status_list = engine_service.GetEngineInfoList();

  tabulate::Table table;
  table.format().font_color(tabulate::Color::green);
  table.add_row(
      {"(Index)", "name", "description", "version", "product name", "status"});
  for (int i = 0; i < status_list.size(); i++) {
    auto status = status_list[i];
    std::string index = std::to_string(i + 1);
    table.add_row({index, status.name, status.description, status.version,
                   status.product_name, status.status});
  }

  for (int i = 0; i < 6; i++) {
    table[0][i]
        .format()
        .font_color(tabulate::Color::white)  // Set font color
        .font_style({tabulate::FontStyle::bold})
        .font_align(tabulate::FontAlign::center);
  }
  for (int i = 1; i < 4; i++) {
    table[i][0]
        .format()
        .font_color(tabulate::Color::white)  // Set font color
        .font_align(tabulate::FontAlign::center);
  }

  std::cout << table << std::endl;
  return true;
}
};  // namespace commands
