#pragma once

#include <trantor/utils/Logger.h>
#include <filesystem>

#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "services/download_service.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/modellist_utils.h"

namespace model_callback_utils {

inline void ParseGguf(const DownloadItem& ggufDownloadItem,
                      std::optional<std::string> author = nullptr) {
  config::GGUFHandler gguf_handler;
  config::YamlHandler yaml_handler;
  gguf_handler.Parse(ggufDownloadItem.localPath.string());
  config::ModelConfig model_config = gguf_handler.GetModelConfig();
  model_config.id =
      ggufDownloadItem.localPath.parent_path().filename().string();
  model_config.files = {ggufDownloadItem.localPath.string()};
  yaml_handler.UpdateModelConfig(model_config);

  auto yaml_path{ggufDownloadItem.localPath};
  auto yaml_name = yaml_path.replace_extension(".yml");

  if (!std::filesystem::exists(yaml_path)) {
    yaml_handler.WriteYamlFile(yaml_path.string());
  }

  auto url_obj = url_parser::FromUrlString(ggufDownloadItem.downloadUrl);
  auto branch = url_obj.pathParams[3];
  CTL_INF("Adding model to modellist with branch: " << branch);

  auto author_id = author.has_value() ? author.value() : "cortexso";
  modellist_utils::ModelListUtils modellist_utils_obj;
  modellist_utils::ModelEntry model_entry{
      .model_id = model_config.id,
      .author_repo_id = author_id,
      .branch_name = branch,
      .path_to_model_yaml = yaml_name.string(),
      .model_alias = model_config.id,
      .status = modellist_utils::ModelStatus::READY};
  modellist_utils_obj.AddModelEntry(model_entry);
}

inline void DownloadModelCb(const DownloadTask& finishedTask) {
  const DownloadItem* model_yml_di = nullptr;
  const DownloadItem* gguf_di = nullptr;
  auto need_parse_gguf = true;

  for (const auto& item : finishedTask.items) {
    if (item.localPath.filename().string() == "model.yml") {
      model_yml_di = &item;
    }
    if (item.localPath.extension().string() == ".gguf") {
      gguf_di = &item;
    }
    if (item.downloadUrl.find("cortexso") != std::string::npos) {
      // if downloading from cortexso, we dont need to parse gguf
      need_parse_gguf = false;
    }
  }

  if (need_parse_gguf && gguf_di != nullptr) {
    ParseGguf(*gguf_di);
  }

  if (model_yml_di != nullptr) {
    auto url_obj = url_parser::FromUrlString(model_yml_di->downloadUrl);
    auto branch = url_obj.pathParams[3];
    CTL_INF("Adding model to modellist with branch: " << branch);
    config::YamlHandler yaml_handler;
    yaml_handler.ModelConfigFromFile(model_yml_di->localPath.string());
    auto mc = yaml_handler.GetModelConfig();

    modellist_utils::ModelListUtils modellist_utils_obj;
    modellist_utils::ModelEntry model_entry{
        .model_id = mc.name,
        .author_repo_id = "cortexso",
        .branch_name = branch,
        .path_to_model_yaml = model_yml_di->localPath.string(),
        .model_alias = mc.name,
        .status = modellist_utils::ModelStatus::READY};
    modellist_utils_obj.AddModelEntry(model_entry);
  }
}
}  // namespace model_callback_utils
