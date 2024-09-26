#include "model_list_cmd.h"
#include <filesystem>
#include <iostream>
#include <tabulate/table.hpp>
#include <vector>
#include "config/yaml_config.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "database/models.h"

namespace commands {

void ModelListCmd::Exec() {
  auto models_path = file_manager_utils::GetModelsContainerPath();
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;
  tabulate::Table table;

  table.add_row({"(Index)", "ID", "model alias", "engine", "version"});
  table.format().font_color(tabulate::Color::green);
  int count = 0;
  // Iterate through directory

  try {
    auto list_entry = modellist_handler.LoadModelList();
    for (const auto& model_entry : list_entry) {
      // auto model_entry = modellist_handler.GetModelInfo(model_handle);
      try {
        count += 1;
        yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
        auto model_config = yaml_handler.GetModelConfig();
        table.add_row({std::to_string(count), model_entry.model_id,
                       model_entry.model_alias, model_config.engine,
                       model_config.version});
        yaml_handler.Reset();
      } catch (const std::exception& e) {
        CTL_ERR("Fail to get list model information: " + std::string(e.what()));
      }
    }
  } catch (const std::exception& e) {
    CTL_ERR("Fail to get list model information: " + std::string(e.what()));
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
