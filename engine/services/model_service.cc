#include "model_service.h"
#include <filesystem>
#include <iostream>
#include "commands/cmd_info.h"
#include "utils/cortexso_parser.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/model_callback_utils.h"
#include "utils/url_parser.h"

void ModelService::DownloadModel(const std::string& input) {
  if (input.empty()) {
    throw std::runtime_error(
        "Input must be Cortex Model Hub handle or HuggingFace url!");
  }

  // case input is a direct url
  auto url_obj = url_parser::FromUrlString(input);
  // TODO: handle case user paste url from cortexso
  if (url_obj.protocol == "https") {
    if (url_obj.host != kHuggingFaceHost) {
      CLI_LOG("Only huggingface.co is supported for now");
      return;
    }
    return DownloadModelByDirectUrl(input);
  } else {
    commands::CmdInfo ci(input);
    return DownloadModelFromCortexso(ci.model_name, ci.branch);
  }
}

std::optional<config::ModelConfig> ModelService::GetDownloadedModel(
    const std::string& modelId) const {
  auto models_path = file_manager_utils::GetModelsContainerPath();
  if (!std::filesystem::exists(models_path) ||
      !std::filesystem::is_directory(models_path)) {
    return std::nullopt;
  }

  for (const auto& entry : std::filesystem::directory_iterator(models_path)) {
    if (entry.is_regular_file() &&
        entry.path().filename().string() == modelId &&
        entry.path().extension() == ".yaml") {
      try {
        config::YamlHandler handler;
        handler.ModelConfigFromFile(entry.path().string());
        auto model_conf = handler.GetModelConfig();
        return model_conf;
      } catch (const std::exception& e) {
        LOG_ERROR << "Error reading yaml file '" << entry.path().string()
                  << "': " << e.what();
      }
    }
  }
  return std::nullopt;
}

void ModelService::DownloadModelByDirectUrl(const std::string& url) {
  // check for malformed url
  // question: What if the url is from cortexso itself
  // answer: then route to download from cortexso
  auto url_obj = url_parser::FromUrlString(url);

  if (url_obj.host == kHuggingFaceHost) {
    // goto hugging face parser to normalize the url
    // loop through path params, replace blob to resolve if any
    if (url_obj.pathParams[2] == "blob") {
      url_obj.pathParams[2] = "resolve";
    }
  }

  // should separate this function out
  auto model_id{url_obj.pathParams[1]};
  auto file_name{url_obj.pathParams.back()};

  auto local_path =
      file_manager_utils::GetModelsContainerPath() / model_id / model_id;

  try {
    std::filesystem::create_directories(local_path.parent_path());
  } catch (const std::filesystem::filesystem_error& e) {
    // if file exist, remove it
    std::filesystem::remove(local_path.parent_path());
    std::filesystem::create_directories(local_path.parent_path());
  }

  auto download_url = url_parser::FromUrl(url_obj);
  // this assume that the model being downloaded is a single gguf file
  auto downloadTask{DownloadTask{.id = url_obj.pathParams.back(),
                                 .type = DownloadType::Model,
                                 .items = {DownloadItem{
                                     .id = url_obj.pathParams.back(),
                                     .downloadUrl = download_url,
                                     .localPath = local_path,
                                 }}}};

  auto on_finished = [](const DownloadTask& finishedTask) {
    std::cout << "Download success" << std::endl;
    auto gguf_download_item = finishedTask.items[0];
    model_callback_utils::ParseGguf(gguf_download_item);
  };

  download_service_.AddDownloadTask(downloadTask, on_finished);
}

void ModelService::DownloadModelFromCortexso(const std::string& name,
                                             const std::string& branch) {
  auto downloadTask = cortexso_parser::getDownloadTask(name, branch);
  if (downloadTask.has_value()) {
    DownloadService().AddDownloadTask(downloadTask.value(),
                                      model_callback_utils::DownloadModelCb);
    CTL_INF("Download finished");
  } else {
    CTL_ERR("Model not found");
  }
}
