#include "model_del_cmd.h"
#include "cmd_info.h"
#include "config/yaml_config.h"
#include "utils/file_manager_utils.h"
#include "database/models.h"

namespace commands {
bool ModelDelCmd::Exec(const std::string& model_handle) {
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;

  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
    auto mc = yaml_handler.GetModelConfig();
    // Remove yaml file
    std::filesystem::remove(model_entry.path_to_model_yaml);
    // Remove model files if they are not imported locally
    if (model_entry.branch_name != "imported") {
      if (mc.files.size() > 0) {
        if (mc.engine == "cortex.llamacpp") {
          for (auto& file : mc.files) {
            std::filesystem::path gguf_p(file);
            std::filesystem::remove(gguf_p);
          }
        } else {
          std::filesystem::path f(mc.files[0]);
          std::filesystem::remove_all(f);
        }
      } else {
        CTL_WRN("model config files are empty!");
      }
    }

    // update model.list
    if (modellist_handler.DeleteModelEntry(model_handle)) {
      CLI_LOG("The model " << model_handle << " was deleted");
      return true;
    } else {
      CTL_ERR("Could not delete model: " << model_handle);
      return false;
    }
  } catch (const std::exception& e) {
    CLI_LOG("Fail to delete model with ID '" + model_handle + "': " + e.what());
    false;
  }
}
}  // namespace commands