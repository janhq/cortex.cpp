#include "model_del_cmd.h"
#include "cmd_info.h"
#include "config/yaml_config.h"
#include "utils/file_manager_utils.h"

namespace commands {
bool ModelDelCmd::Exec(const std::string& model_id) {
  // TODO this implentation may be changed after we have a decision
  // on https://github.com/janhq/cortex.cpp/issues/1154 but the logic should be similar
  CmdInfo ci(model_id);
  std::string model_file =
      ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
  auto models_path = file_manager_utils::GetModelsContainerPath();
  if (std::filesystem::exists(models_path) &&
      std::filesystem::is_directory(models_path)) {
    // Iterate through directory
    for (const auto& entry : std::filesystem::directory_iterator(models_path)) {
      if (entry.is_regular_file() && entry.path().extension() == ".yaml") {
        try {
          config::YamlHandler handler;
          handler.ModelConfigFromFile(entry.path().string());
          auto cfg = handler.GetModelConfig();
          if (entry.path().stem().string() == model_file) {
            // Delete data
            if (cfg.files.size() > 0) {
              std::filesystem::path f(cfg.files[0]);
              auto rel = std::filesystem::relative(f, models_path);
              // Only delete model data if it is stored in our models folder
              if (!rel.empty()) {
                if (cfg.engine == "cortex.llamacpp") {
                  std::filesystem::remove_all(f.parent_path());
                } else {
                  std::filesystem::remove_all(f);
                }
              }
            }

            // Delete yaml file
            std::filesystem::remove(entry);
            CLI_LOG("The model " << model_id << " was deleted");
            return true;
          }
        } catch (const std::exception& e) {
          CTL_WRN("Error reading yaml file '" << entry.path().string()
                                              << "': " << e.what());
          return false;
        }
      }
    }
  }

  CLI_LOG("Model does not exist: " << model_id);

  return false;
}
}  // namespace commands