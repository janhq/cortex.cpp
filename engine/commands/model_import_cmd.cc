#include "model_import_cmd.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "trantor/utils/Logger.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/modellist_utils.h"

namespace commands {

ModelImportCmd::ModelImportCmd(std::string model_handle, std::string model_path)
    : model_handle_(std::move(model_handle)),
      model_path_(std::move(model_path)) {}

void ModelImportCmd::Exec() {
  config::GGUFHandler gguf_handler;
  config::YamlHandler yaml_handler;
  modellist_utils::ModelListUtils modellist_utils_obj;

  std::string model_yaml_path = (file_manager_utils::GetModelsContainerPath() /
                                 std::filesystem::path("imported") /
                                 std::filesystem::path(model_handle_ + ".yml"))
                                    .string();
  modellist_utils::ModelEntry model_entry{
      model_handle_,   "local",       "imported",
      model_yaml_path, model_handle_, modellist_utils::ModelStatus::READY};
  try {
    std::filesystem::create_directories(
        std::filesystem::path(model_yaml_path).parent_path());
    gguf_handler.Parse(model_path_);
    config::ModelConfig model_config = gguf_handler.GetModelConfig();
    model_config.files.push_back(model_path_);
    model_config.name = model_handle_;
    yaml_handler.UpdateModelConfig(model_config);

    if(modellist_utils_obj.AddModelEntry(model_entry)){
        yaml_handler.WriteYamlFile(model_yaml_path);
        CLI_LOG("Model is imported successfully!");
    }
    else{
        CLI_LOG("Fail to import model, model_id '"+model_handle_+"' already exists!" );
    }
    
  } catch (const std::exception& e) {
    std::remove(model_yaml_path.c_str());
    CTL_ERR("Error importing model '" << model_path_ << "' with model_id '"
                                      << model_handle_ << "': " << e.what());
  }
}
}  // namespace commands