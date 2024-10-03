#include "model_import_cmd.h"
#include <filesystem>
#include <vector>
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "database/models.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {

ModelImportCmd::ModelImportCmd(std::string model_handle, std::string model_path)
    : model_handle_(std::move(model_handle)),
      model_path_(std::move(model_path)) {}

void ModelImportCmd::Exec() {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  config::GGUFHandler gguf_handler;
  config::YamlHandler yaml_handler;
  cortex::db::Models modellist_utils_obj;

  std::string model_yaml_path = (file_manager_utils::GetModelsContainerPath() /
                                 std::filesystem::path("imported") /
                                 std::filesystem::path(model_handle_ + ".yml"))
                                    .string();
  try {
    // Use relative path for model_yaml_path. In case of import, we use absolute path for model
    auto yaml_rel_path =
        fmu::Subtract(fs::path(model_yaml_path), fmu::GetCortexDataPath());
    cortex::db::ModelEntry model_entry{model_handle_, "local", "imported",
                                       yaml_rel_path.string(), model_handle_};

    std::filesystem::create_directories(
        std::filesystem::path(model_yaml_path).parent_path());
    gguf_handler.Parse(model_path_);
    auto model_config = gguf_handler.GetModelConfig();
    model_config.files.push_back(model_path_);
    model_config.model = model_handle_;
    yaml_handler.UpdateModelConfig(model_config);

    if (modellist_utils_obj.AddModelEntry(model_entry).value()) {
      yaml_handler.WriteYamlFile(model_yaml_path);
      CLI_LOG("Model is imported successfully!");
    } else {
      CLI_LOG("Fail to import model, model_id '" + model_handle_ +
              "' already exists!");
    }

  } catch (const std::exception& e) {
    // don't need to remove yml file here, because it's written only if model entry is successfully added,
    // remove file here can make it fail with edge case when user try to import new model with existed model_id
    CLI_LOG("Error importing model path '" + model_path_ + "' with model_id '" +
            model_handle_ + "': " + e.what());
  }
}
}  // namespace commands
