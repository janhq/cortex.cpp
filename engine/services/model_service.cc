#include "model_service.h"
#include <filesystem>
#include <iostream>
#include <ostream>
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "utils/cli_selection_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "database/models.h"
#include "utils/result.hpp"
#include "utils/string_utils.h"

namespace {
void ParseGguf(const DownloadItem& ggufDownloadItem,
               std::optional<std::string> author) {

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
  cortex::db::Models modellist_utils_obj;
  cortex::db::ModelEntry model_entry{
      .model_id = ggufDownloadItem.id,
      .author_repo_id = author_id,
      .branch_name = branch,
      .path_to_model_yaml = yaml_name.string(),
      .model_alias = ggufDownloadItem.id,
      .status = cortex::db::ModelStatus::READY};
  modellist_utils_obj.AddModelEntry(model_entry, true);
}

cpp::result<DownloadTask, std::string> GetDownloadTask(
    const std::string& modelId, const std::string& branch = "main") {
  using namespace nlohmann;
  url_parser::Url url = {
      .protocol = "https",
      .host = ModelService::kHuggingFaceHost,
      .pathParams = {"api", "models", "cortexso", modelId, "tree", branch}};

  httplib::Client cli(url.GetProtocolAndHost());
  auto res = cli.Get(url.GetPathAndQuery());
  if (res->status != httplib::StatusCode::OK_200) {
    auto err = res.error();
    return cpp::fail("HTTP error: " + httplib::to_string(err));
  }
  auto jsonResponse = json::parse(res->body);

  std::vector<DownloadItem> download_items{};
  auto model_container_path = file_manager_utils::GetModelsContainerPath() /
                              "cortex.so" / modelId / branch;
  file_manager_utils::CreateDirectoryRecursively(model_container_path.string());

  for (const auto& [key, value] : jsonResponse.items()) {
    auto path = value["path"].get<std::string>();
    if (path == ".gitattributes" || path == ".gitignore" ||
        path == "README.md") {
      continue;
    }
    url_parser::Url download_url = {
        .protocol = "https",
        .host = ModelService::kHuggingFaceHost,
        .pathParams = {"cortexso", modelId, "resolve", branch, path}};

    auto local_path = model_container_path / path;
    download_items.push_back(
        DownloadItem{.id = path,
                     .downloadUrl = download_url.ToFullPath(),
                     .localPath = local_path});
  }

  DownloadTask download_tasks{
      .id = branch == "main" ? modelId : modelId + "-" + branch,
      .type = DownloadType::Model,
      .items = download_items};

  return download_tasks;
}
}  // namespace

cpp::result<std::string, std::string> ModelService::DownloadModel(
    const std::string& input, bool async) {
  if (input.empty()) {
    return cpp::fail(
        "Input must be Cortex Model Hub handle or HuggingFace url!");
  }

  if (string_utils::StartsWith(input, "https://")) {
    // TODO: better name, for example handle url
    return HandleUrl(input, async);
  }

  if (input.find(":") != std::string::npos) {
    auto parsed = string_utils::SplitBy(input, ":");
    if (parsed.size() != 2) {
      return cpp::fail("Invalid model handle: " + input);
    }
    return DownloadModelFromCortexso(parsed[0], parsed[1], false);
  }

  if (input.find("/") != std::string::npos) {
    auto parsed = string_utils::SplitBy(input, "/");
    if (parsed.size() != 2) {
      return cpp::fail("Invalid model handle: " + input);
    }

    auto author = parsed[0];
    auto model_name = parsed[1];
    if (author == "cortexso") {
      return HandleCortexsoModel(model_name);
    }

    return DownloadHuggingFaceGgufModel(author, model_name, std::nullopt,
                                        async);
  }

  return HandleCortexsoModel(input);
}

cpp::result<std::string, std::string> ModelService::HandleCortexsoModel(
    const std::string& modelName) {
  auto branches =
      huggingface_utils::GetModelRepositoryBranches("cortexso", modelName);
  if (branches.has_error()) {
    return cpp::fail(branches.error());
  }

  std::vector<std::string> options{};
  for (const auto& branch : branches.value()) {
    if (branch.name != "main") {
      options.emplace_back(branch.name);
    }
  }
  if (options.empty()) {
    return cpp::fail("No variant available");
  }
  auto selection = cli_selection_utils::PrintSelection(options);
  return DownloadModelFromCortexso(modelName, selection.value());
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

cpp::result<std::string, std::string> ModelService::HandleUrl(
    const std::string& url, bool async) {
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
    // TODO: try to get the branch
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

  if (async) {
    auto result =
        download_service_.AddAsyncDownloadTask(downloadTask, on_finished);

    if (result.has_error()) {
      CTL_ERR(result.error());
      return cpp::fail(result.error());
    }
    return unique_model_id;
  } else {
    auto result = download_service_.AddDownloadTask(downloadTask, on_finished);
    if (result.has_error()) {
      CTL_ERR(result.error());
      return cpp::fail(result.error());
    }
    return unique_model_id;
  }
}

cpp::result<std::string, std::string> ModelService::DownloadModelFromCortexso(
    const std::string& name, const std::string& branch, bool async) {

  auto download_task = GetDownloadTask(name, branch);
  if (download_task.has_error()) {
    return cpp::fail(download_task.error());
  }

  std::string model_id{name + ":" + branch};
  auto on_finished = [&](const DownloadTask& finishedTask) {
    const DownloadItem* model_yml_item = nullptr;
    auto need_parse_gguf = true;

    for (const auto& item : finishedTask.items) {
      if (item.localPath.filename().string() == "model.yml") {
        model_yml_item = &item;
      }
    }

    if (model_yml_item == nullptr) {
      CTL_WRN("model.yml not found in the downloaded files for " + model_id);
      return;
    }
    auto url_obj = url_parser::FromUrlString(model_yml_item->downloadUrl);
    CTL_INF("Adding model to modellist with branch: " << branch);
    config::YamlHandler yaml_handler;
    yaml_handler.ModelConfigFromFile(model_yml_item->localPath.string());
    auto mc = yaml_handler.GetModelConfig();

    cortex::db::Models modellist_utils_obj;
    cortex::db::ModelEntry model_entry{
        .model_id = model_id,
        .author_repo_id = "cortexso",
        .branch_name = branch,
        .path_to_model_yaml = model_yml_item->localPath.string(),
        .model_alias = model_id,
        .status = cortex::db::ModelStatus::READY};
    modellist_utils_obj.AddModelEntry(model_entry);
  };

  auto result = async ? DownloadService().AddAsyncDownloadTask(
                            download_task.value(), on_finished)
                      : DownloadService().AddDownloadTask(download_task.value(),
                                                          on_finished);

  if (result.has_error()) {
    return cpp::fail(result.error());
  }

  CLI_LOG("Model " << model_id << " downloaded successfully!")
  return model_id;
}

cpp::result<std::string, std::string>
ModelService::DownloadHuggingFaceGgufModel(const std::string& author,
                                           const std::string& modelName,
                                           std::optional<std::string> fileName,
                                           bool async) {
  auto repo_info =
      huggingface_utils::GetHuggingFaceModelRepoInfo(author, modelName);

  if (!repo_info.has_value()) {
    return cpp::fail("Model not found");
  }

  if (!repo_info->gguf.has_value()) {
    return cpp::fail(
        "Not a GGUF model. Currently, only GGUF single file is "
        "supported.");
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
  return HandleUrl(download_url);
}

cpp::result<void, std::string> ModelService::DeleteModel(
    const std::string& model_handle) {

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
      return {};
    } else {
      return cpp::fail("Could not delete model: " + model_handle);
    }
  } catch (const std::exception& e) {
    return cpp::fail("Fail to delete model with ID '" + model_handle +
                     "': " + e.what());
  }
}
