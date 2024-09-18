#include "model_service.h"
#include <filesystem>
#include <iostream>
#include <ostream>
#include "utils/cli_selection_utils.h"
#include "utils/cortexso_parser.h"
#include "utils/file_manager_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/model_callback_utils.h"
#include "utils/string_utils.h"

void ModelService::DownloadModel(const std::string& input) {
  if (input.empty()) {
    throw std::runtime_error(
        "Input must be Cortex Model Hub handle or HuggingFace url!");
  }

  if (string_utils::StartsWith(input, "https://")) {
    return DownloadModelByDirectUrl(input);
  }

  if (input.find("/") != std::string::npos) {
    auto parsed = string_utils::SplitBy(input, "/");
    if (parsed.size() != 2) {
      throw std::runtime_error("Invalid model handle: " + input);
    }

    auto author = parsed[0];
    auto model_name = parsed[1];
    if (author == "cortexso") {
      return DownloadModelByModelName(model_name);
    }

    DownloadHuggingFaceGgufModel(author, model_name, std::nullopt);
    CLI_LOG("Model " << model_name << " downloaded successfully!")
    return;
  }

  return DownloadModelByModelName(input);
}

void ModelService::DownloadModelByModelName(const std::string& modelName) {
  try {
    auto branches =
        huggingface_utils::GetModelRepositoryBranches("cortexso", modelName);
    std::vector<std::string> options{};
    for (const auto& branch : branches) {
      if (branch.name != "main") {
        options.emplace_back(branch.name);
      }
    }
    if (options.empty()) {
      CLI_LOG("No variant found");
      return;
    }
    auto selection = cli_selection_utils::PrintSelection(options);
    DownloadModelFromCortexso(modelName, selection.value());
  } catch (const std::runtime_error& e) {
    CLI_LOG("Error downloading model, " << e.what());
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
  auto url_obj = url_parser::FromUrlString(url);

  if (url_obj.host == kHuggingFaceHost) {
    if (url_obj.pathParams[2] == "blob") {
      url_obj.pathParams[2] = "resolve";
    }
  }

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
  auto downloadTask{DownloadTask{.id = model_id,
                                 .type = DownloadType::Model,
                                 .items = {DownloadItem{
                                     .id = url_obj.pathParams.back(),
                                     .downloadUrl = download_url,
                                     .localPath = local_path,
                                 }}}};

  auto on_finished = [](const DownloadTask& finishedTask) {
    CLI_LOG("Model " << finishedTask.id << " downloaded successfully!")
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
    CLI_LOG("Model " << name << " downloaded successfully!")
  } else {
    CTL_ERR("Model not found");
  }
}

void ModelService::DownloadHuggingFaceGgufModel(
    const std::string& author, const std::string& modelName,
    std::optional<std::string> fileName) {
  auto repo_info =
      huggingface_utils::GetHuggingFaceModelRepoInfo(author, modelName);
  if (!repo_info.has_value()) {
    // throw is better?
    CTL_ERR("Model not found");
    return;
  }

  if (!repo_info->gguf.has_value()) {
    throw std::runtime_error(
        "Not a GGUF model. Currently, only GGUF single file is supported.");
  }

  std::vector<std::string> options{};
  for (const auto& sibling : repo_info->siblings) {
    if (string_utils::EndsWith(sibling.rfilename, ".gguf")) {
      options.push_back(sibling.rfilename);
    }
  }
  auto selection = cli_selection_utils::PrintSelection(options);
  std::cout << "Selected: " << selection.value() << std::endl;

  auto download_url = huggingface_utils::GetDownloadableUrl(author, modelName,
                                                            selection.value());
  DownloadModelByDirectUrl(download_url);
}
