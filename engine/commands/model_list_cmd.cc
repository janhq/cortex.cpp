#include "model_list_cmd.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include "utils/cortex_utils.h"
#include "config/yaml_config.h"
#include "trantor/utils/Logger.h"
namespace commands {

void ModelListCmd::Exec() {
  if (std::filesystem::exists(cortex_utils::models_folder) &&
      std::filesystem::is_directory(cortex_utils::models_folder)) {
    // Iterate through directory
    for (const auto& entry :
         std::filesystem::directory_iterator(cortex_utils::models_folder)) {
      if (entry.is_regular_file() && entry.path().extension() == ".yaml") {
        try {
            config::YamlHandler handler;
            handler.ModelConfigFromFile(entry.path().string());
            std::cout<<"Model ID: "<< entry.path().stem().string() <<", Engine: "<< handler.GetModelConfig().engine <<std::endl;

        } catch (const std::exception& e) {
          LOG_ERROR << "Error reading yaml file '" << entry.path().string()
                    << "': " << e.what();
        }
      }
    }
  }
}
};  // namespace commands