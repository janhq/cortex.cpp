#include "model_get_cmd.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>
#include "cmd_info.h"
#include "config/yaml_config.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "database/models.h"

namespace commands {

void ModelGetCmd::Exec(const std::string& model_handle) {
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;
  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
    auto model_config = yaml_handler.GetModelConfig();

    std::cout << model_config.ToString() << std::endl;

  } catch (const std::exception& e) {
    CLI_LOG("Fail to get model information with ID '" + model_handle +
            "': " + e.what());
  }
}

}  // namespace commands