#pragma once
#include <trantor/utils/Logger.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "utils/file_manager_utils.h"

namespace model_callback_utils {
inline void DownloadModelCb(const std::string& path, bool need_parse_gguf) {

  std::filesystem::path path_obj(path);
  std::string filename(path_obj.filename().string());
  //TODO: handle many cases of downloaded items from other sources except cortexso.
  if (filename.compare("model.yml") == 0) {
    config::YamlHandler handler;
    handler.ModelConfigFromFile(path);
    config::ModelConfig model_config = handler.GetModelConfig();
    model_config.id = path_obj.parent_path().filename().string();

    LOG_INFO << "Updating model config in " << path;
    handler.UpdateModelConfig(model_config);
    handler.WriteYamlFile(path_obj.parent_path().parent_path().string() + "/" +
                          model_config.id + ".yaml");
  }
  // currently, only handle downloaded model with only 1 .gguf file
  // TODO: handle multipart gguf file or different model in 1 repo.
  else if (path_obj.extension().string().compare(".gguf") == 0) {
    if(!need_parse_gguf) return;    
    config::GGUFHandler gguf_handler;
    config::YamlHandler yaml_handler;
    gguf_handler.Parse(path);
    config::ModelConfig model_config = gguf_handler.GetModelConfig();
    model_config.id = path_obj.parent_path().filename().string();
    model_config.files = {path};
    yaml_handler.UpdateModelConfig(model_config);
    std::string yml_path(path_obj.parent_path().parent_path().string() + "/" +
                         model_config.id + ".yaml");
    std::string yaml_path(path_obj.parent_path().string() + "/model.yml");
    if (!std::filesystem::exists(yml_path)) {  // if model.yml doesn't exsited
      yaml_handler.WriteYamlFile(yml_path);
    }
    if (!std::filesystem::exists(
            yaml_path)) {  // if <model_id>.yaml doesn't exsited
      yaml_handler.WriteYamlFile(yaml_path);
    }
  }
}
}  // namespace model_callback_utils