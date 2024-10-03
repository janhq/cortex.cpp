#include "model_get_cmd.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>
#include "config/yaml_config.h"
#include "database/models.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {

void ModelGetCmd::Exec(const std::string& model_handle) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;
  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    if (model_entry.has_error()) {
      CLI_LOG("Error: " + model_entry.error());
      return;
    }
    yaml_handler.ModelConfigFromFile(
        fmu::GetAbsolutePath(fmu::GetCortexDataPath(),
                             fs::path(model_entry.value().path_to_model_yaml))
            .string());
    auto model_config = yaml_handler.GetModelConfig();

    std::cout << model_config.ToString() << std::endl;

  } catch (const std::exception& e) {
    CLI_LOG("Fail to get model information with ID '" + model_handle +
            "': " + e.what());
  }
}

}  // namespace commands