#include "model_list_cmd.h"
#include <filesystem>
#include <iostream>
#include <tabulate/table.hpp>
#include <vector>
#include "config/yaml_config.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {

void ModelListCmd::Exec() {
  auto models_path = file_manager_utils::GetModelsContainerPath();
  if (std::filesystem::exists(models_path) &&
      std::filesystem::is_directory(models_path)) {
    tabulate::Table table;

    table.add_row({"(Index)", "ID", "engine", "version"});
    table.format().font_color(tabulate::Color::green);
    int count = 0;
    // Iterate through directory
    for (const auto& entry : std::filesystem::directory_iterator(models_path)) {
      if (entry.is_regular_file() && entry.path().extension() == ".yaml") {
        try {
          count += 1;
          config::YamlHandler handler;
          handler.ModelConfigFromFile(entry.path().string());
          const auto& model_config = handler.GetModelConfig();
          table.add_row({std::to_string(count), model_config.id,
                         model_config.engine, model_config.version});
        } catch (const std::exception& e) {
          CTL_ERR("Error reading yaml file '" << entry.path().string()
                                              << "': " << e.what());
        }
      }
    }
    for (int i = 0; i < 4; i++) {
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
      table[i][3]  //version value
          .format()
          .font_align(tabulate::FontAlign::center);
    }
    std::cout << table << std::endl;
  }
}
};  // namespace commands
