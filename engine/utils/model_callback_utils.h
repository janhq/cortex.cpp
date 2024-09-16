#pragma once

#include <trantor/utils/Logger.h>
#include <filesystem>

#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "services/download_service.h"
#include "utils/logging_utils.h"

namespace model_callback_utils {
inline void WriteYamlOutput(const DownloadItem& modelYmlDownloadItem) {
  config::YamlHandler handler;
  handler.ModelConfigFromFile(modelYmlDownloadItem.localPath.string());
  config::ModelConfig model_config = handler.GetModelConfig();
  model_config.id =
      modelYmlDownloadItem.localPath.parent_path().filename().string();

  CTL_INF("Updating model config in "
          << modelYmlDownloadItem.localPath.string());
  handler.UpdateModelConfig(model_config);
  std::string yaml_filename{model_config.id + ".yaml"};
  std::filesystem::path yaml_output =
      modelYmlDownloadItem.localPath.parent_path().parent_path() /
      yaml_filename;
  handler.WriteYamlFile(yaml_output.string());
}

inline void ParseGguf(const DownloadItem& ggufDownloadItem) {
  config::GGUFHandler gguf_handler;
  config::YamlHandler yaml_handler;
  gguf_handler.Parse(ggufDownloadItem.localPath.string());
  config::ModelConfig model_config = gguf_handler.GetModelConfig();
  model_config.id =
      ggufDownloadItem.localPath.parent_path().filename().string();
  model_config.files = {ggufDownloadItem.localPath.string()};
  yaml_handler.UpdateModelConfig(model_config);

  std::string yaml_filename{model_config.id + ".yaml"};
  std::filesystem::path yaml_output =
      ggufDownloadItem.localPath.parent_path().parent_path() / yaml_filename;
  std::filesystem::path yaml_path(ggufDownloadItem.localPath.parent_path() /
                                  "model.yml");
  if (!std::filesystem::exists(yaml_output)) {  // if model.yml doesn't exist
    yaml_handler.WriteYamlFile(yaml_output.string());
  }
  if (!std::filesystem::exists(yaml_path)) {
    yaml_handler.WriteYamlFile(yaml_path.string());
  }
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

  if (model_yml_di != nullptr) {
    WriteYamlOutput(*model_yml_di);
  }

  if (need_parse_gguf && gguf_di != nullptr) {
    ParseGguf(*gguf_di);
  }
}
}  // namespace model_callback_utils
