#include "model_service.h"
#include <filesystem>
#include <iostream>
#include <ostream>
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "utils/cli_selection_utils.h"
#include "utils/cortexso_parser.h"
#include "utils/file_manager_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/modellist_utils.h"
#include "utils/string_utils.h"

std::optional<std::string> ModelService::DownloadModel(
    const std::string& input) {
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

    CLI_LOG("Model " << model_name << " downloaded successfully!")
    return DownloadHuggingFaceGgufModel(author, model_name, std::nullopt);
  }

  return DownloadModelByModelName(input);
}

std::optional<std::string> ModelService::DownloadModelByModelName(
    const std::string& modelName) {
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
      return std::nullopt;
    }
    auto selection = cli_selection_utils::PrintSelection(options);
    return DownloadModelFromCortexso(modelName, selection.value());
  } catch (const std::runtime_error& e) {
    CLI_LOG("Error downloading model, " << e.what());
    return std::nullopt;
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

std::optional<std::string> ModelService::DownloadModelByDirectUrl(
    const std::string& url) {
  auto url_obj = url_parser::FromUrlString(url);

  if (url_obj.host == kHuggingFaceHost) {
    if (url_obj.pathParams[2] == "blob") {
      url_obj.pathParams[2] = "resolve";
    }
  }
  auto author{url_obj.pathParams[0]};
  auto model_id{url_obj.pathParams[1]};
  auto file_name{url_obj.pathParams.back()};

  if (author == "cortexso") {
    return DownloadModelFromCortexso(model_id);
  }

  std::string huggingFaceHost{kHuggingFaceHost};
  std::string unique_model_id{huggingFaceHost + "/" + author + "/" + model_id +
                              "/" + file_name};
  auto local_path{file_manager_utils::GetModelsContainerPath() /
                  "huggingface.co" / author / model_id / file_name};

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
                                     .id = unique_model_id,
                                     .downloadUrl = download_url,
                                     .localPath = local_path,
                                 }}}};

  auto on_finished = [&](const DownloadTask& finishedTask) {
    CLI_LOG("Model " << finishedTask.id << " downloaded successfully!")
    auto gguf_download_item = finishedTask.items[0];
    ParseGguf(gguf_download_item, author);
  };

  download_service_.AddDownloadTask(downloadTask, on_finished);
  return unique_model_id;
}

std::optional<std::string> ModelService::DownloadModelFromCortexso(
    const std::string& name, const std::string& branch) {

  auto downloadTask = cortexso_parser::getDownloadTask(name, branch);
  if (downloadTask.has_value()) {
    std::string model_id{name + ":" + branch};
    DownloadService().AddDownloadTask(
        downloadTask.value(), [&](const DownloadTask& finishedTask) {
          const DownloadItem* model_yml_item = nullptr;
          auto need_parse_gguf = true;

          for (const auto& item : finishedTask.items) {
            if (item.localPath.filename().string() == "model.yml") {
              model_yml_item = &item;
            }
          }

          if (model_yml_item != nullptr) {
            auto url_obj =
                url_parser::FromUrlString(model_yml_item->downloadUrl);
            CTL_INF("Adding model to modellist with branch: " << branch);
            config::YamlHandler yaml_handler;
            yaml_handler.ModelConfigFromFile(
                model_yml_item->localPath.string());
            auto mc = yaml_handler.GetModelConfig();

            modellist_utils::ModelListUtils modellist_utils_obj;
            modellist_utils::ModelEntry model_entry{
                .model_id = model_id,
                .author_repo_id = "cortexso",
                .branch_name = branch,
                .path_to_model_yaml = model_yml_item->localPath.string(),
                .model_alias = model_id,
                .status = modellist_utils::ModelStatus::READY};
            modellist_utils_obj.AddModelEntry(model_entry);
          }
        });

    CLI_LOG("Model " << model_id << " downloaded successfully!")
    return model_id;
  } else {
    CTL_ERR("Model not found");
    return std::nullopt;
  }
}

std::optional<std::string> ModelService::DownloadHuggingFaceGgufModel(
    const std::string& author, const std::string& modelName,
    std::optional<std::string> fileName) {
  auto repo_info =
      huggingface_utils::GetHuggingFaceModelRepoInfo(author, modelName);
  if (!repo_info.has_value()) {
    // throw is better?
    CTL_ERR("Model not found");
    return std::nullopt;
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
  return DownloadModelByDirectUrl(download_url);
}

void ModelService::ParseGguf(const DownloadItem& ggufDownloadItem,
                             std::optional<std::string> author) const {

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
      .model_id = ggufDownloadItem.id,
      .author_repo_id = author_id,
      .branch_name = branch,
      .path_to_model_yaml = yaml_name.string(),
      .model_alias = ggufDownloadItem.id,
      .status = modellist_utils::ModelStatus::READY};
  modellist_utils_obj.AddModelEntry(model_entry, true);
}
