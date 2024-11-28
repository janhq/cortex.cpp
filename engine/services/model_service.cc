#include "model_service.h"
#include <filesystem>
#include <iostream>
#include <optional>
#include <ostream>
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "database/models.h"
#include "hardware_service.h"
#include "httplib.h"
#include "utils/cli_selection_utils.h"
#include "utils/cortex_utils.h"
#include "utils/engine_constants.h"
#include "utils/file_manager_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "utils/string_utils.h"

namespace {
void ParseGguf(const DownloadItem& ggufDownloadItem,
               std::optional<std::string> author,
               std::optional<std::string> name,
               std::optional<std::uint64_t> size) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  config::GGUFHandler gguf_handler;
  config::YamlHandler yaml_handler;
  gguf_handler.Parse(ggufDownloadItem.localPath.string());

  config::ModelConfig model_config = gguf_handler.GetModelConfig();
  model_config.id =
      ggufDownloadItem.localPath.parent_path().filename().string();
  // use relative path for files
  auto file_rel_path =
      fmu::ToRelativeCortexDataPath(fs::path(ggufDownloadItem.localPath));
  model_config.files = {file_rel_path.string()};
  model_config.model = ggufDownloadItem.id;
  model_config.name =
      name.has_value() ? name.value() : gguf_handler.GetModelConfig().name;
  model_config.size = size.value_or(0);
  yaml_handler.UpdateModelConfig(model_config);

  auto yaml_path{ggufDownloadItem.localPath};
  auto yaml_name = yaml_path.replace_extension(".yml");

  if (!std::filesystem::exists(yaml_path)) {
    yaml_handler.WriteYamlFile(yaml_path.string());
  }

  auto url_obj = url_parser::FromUrlString(ggufDownloadItem.downloadUrl);
  if (url_obj.has_error()) {
    CTL_WRN("Error parsing url: " << ggufDownloadItem.downloadUrl);
    return;
  }

  auto branch = url_obj->pathParams[3];
  CTL_INF("Adding model to modellist with branch: " << branch);

  auto rel = file_manager_utils::ToRelativeCortexDataPath(yaml_name);
  CTL_INF("path_to_model_yaml: " << rel.string());

  auto author_id = author.has_value() ? author.value() : "cortexso";
  cortex::db::Models modellist_utils_obj;
  cortex::db::ModelEntry model_entry{.model = ggufDownloadItem.id,
                                     .author_repo_id = author_id,
                                     .branch_name = branch,
                                     .path_to_model_yaml = rel.string(),
                                     .model_alias = ggufDownloadItem.id};
  auto result = modellist_utils_obj.AddModelEntry(model_entry, true);
  if (result.has_error()) {
    CTL_WRN("Error adding model to modellist: " + result.error());
  }
}

cpp::result<DownloadTask, std::string> GetDownloadTask(
    const std::string& modelId, const std::string& branch = "main") {
  url_parser::Url url = {
      .protocol = "https",
      .host = kHuggingFaceHost,
      .pathParams = {"api", "models", "cortexso", modelId, "tree", branch}};

  auto result = curl_utils::SimpleGetJson(url.ToFullPath());
  if (result.has_error()) {
    return cpp::fail("Model " + modelId + " not found");
  }

  std::vector<DownloadItem> download_items{};
  auto model_container_path = file_manager_utils::GetModelsContainerPath() /
                              "cortex.so" / modelId / branch;
  file_manager_utils::CreateDirectoryRecursively(model_container_path.string());

  for (const auto& value : result.value()) {
    auto path = value["path"].asString();
    if (path == ".gitattributes" || path == ".gitignore" ||
        path == "README.md") {
      continue;
    }
    url_parser::Url download_url = {
        .protocol = "https",
        .host = kHuggingFaceHost,
        .pathParams = {"cortexso", modelId, "resolve", branch, path}};

    auto local_path = model_container_path / path;
    download_items.push_back(
        DownloadItem{.id = path,
                     .downloadUrl = download_url.ToFullPath(),
                     .localPath = local_path});
  }

  return DownloadTask{.id = branch == "main" ? modelId : modelId + "-" + branch,
                      .type = DownloadType::Model,
                      .items = download_items};
}
}  // namespace

void ModelService::ForceIndexingModelList() {
  CTL_INF("Force indexing model list");

  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;

  auto list_entry = modellist_handler.LoadModelList();
  if (list_entry.has_error()) {
    CTL_ERR("Failed to load model list: " << list_entry.error());
    return;
  }

  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;

  CTL_DBG("Database model size: " + std::to_string(list_entry.value().size()));
  for (const auto& model_entry : list_entry.value()) {
    try {
      yaml_handler.ModelConfigFromFile(
          fmu::ToAbsoluteCortexDataPath(
              fs::path(model_entry.path_to_model_yaml))
              .string());
      auto model_config = yaml_handler.GetModelConfig();
      Json::Value obj = model_config.ToJson();
      yaml_handler.Reset();
    } catch (const std::exception& e) {
      // remove in db
      auto remove_result =
          modellist_handler.DeleteModelEntry(model_entry.model);
      // silently ignore result
    }
  }
}

cpp::result<std::string, std::string> ModelService::DownloadModel(
    const std::string& input) {
  if (input.empty()) {
    return cpp::fail(
        "Input must be Cortex Model Hub handle or HuggingFace url!");
  }

  if (string_utils::StartsWith(input, "https://")) {
    return HandleUrl(input);
  }

  if (input.find(":") != std::string::npos) {
    auto parsed = string_utils::SplitBy(input, ":");
    if (parsed.size() != 2) {
      return cpp::fail("Invalid model handle: " + input);
    }
    return DownloadModelFromCortexso(parsed[0], parsed[1]);
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

    return DownloadHuggingFaceGgufModel(author, model_name, std::nullopt);
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

  auto default_model_branch = huggingface_utils::GetDefaultBranch(modelName);

  cortex::db::Models modellist_handler;
  auto downloaded_model_ids =
      modellist_handler.FindRelatedModel(modelName).value_or(
          std::vector<std::string>{});

  std::vector<std::string> avai_download_opts{};
  for (const auto& branch : branches.value()) {
    if (branch.second.name == "main") {  // main branch only have metadata. skip
      continue;
    }
    auto model_id = modelName + ":" + branch.second.name;
    if (std::find(downloaded_model_ids.begin(), downloaded_model_ids.end(),
                  model_id) !=
        downloaded_model_ids.end()) {  // if downloaded, we skip it
      continue;
    }
    avai_download_opts.emplace_back(model_id);
  }

  if (avai_download_opts.empty()) {
    // TODO: only with pull, we return
    return cpp::fail("No variant available");
  }
  std::optional<std::string> normalized_def_branch = std::nullopt;
  if (default_model_branch.has_value()) {
    normalized_def_branch = modelName + ":" + default_model_branch.value();
  }
  string_utils::SortStrings(downloaded_model_ids);
  string_utils::SortStrings(avai_download_opts);
  auto selection = cli_selection_utils::PrintModelSelection(
      downloaded_model_ids, avai_download_opts, normalized_def_branch);
  if (!selection.has_value()) {
    return cpp::fail("Invalid selection");
  }

  CLI_LOG("Selected: " << selection.value());
  auto branch_name = selection.value().substr(modelName.size() + 1);
  return DownloadModelFromCortexso(modelName, branch_name);
}

std::optional<config::ModelConfig> ModelService::GetDownloadedModel(
    const std::string& modelId) const {

  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;
  auto model_entry = modellist_handler.GetModelInfo(modelId);
  if (!model_entry.has_value()) {
    return std::nullopt;
  }

  try {
    config::YamlHandler yaml_handler;
    namespace fs = std::filesystem;
    namespace fmu = file_manager_utils;
    yaml_handler.ModelConfigFromFile(
        fmu::ToAbsoluteCortexDataPath(
            fs::path(model_entry.value().path_to_model_yaml))
            .string());
    return yaml_handler.GetModelConfig();
  } catch (const std::exception& e) {
    LOG_ERROR << "Error reading yaml file '" << model_entry->path_to_model_yaml
              << "': " << e.what();
    return std::nullopt;
  }
}

cpp::result<DownloadTask, std::string> ModelService::HandleDownloadUrlAsync(
    const std::string& url, std::optional<std::string> temp_model_id,
    std::optional<std::string> temp_name) {
  auto url_obj = url_parser::FromUrlString(url);
  if (url_obj.has_error()) {
    return cpp::fail("Invalid url: " + url);
  }

  if (url_obj->host == kHuggingFaceHost) {
    if (url_obj->pathParams[2] == "blob") {
      url_obj->pathParams[2] = "resolve";
    }
  }
  auto author{url_obj->pathParams[0]};
  auto model_id{url_obj->pathParams[1]};
  auto file_name{url_obj->pathParams.back()};

  if (author == "cortexso") {
    return DownloadModelFromCortexsoAsync(model_id, url_obj->pathParams[3]);
  }

  if (url_obj->pathParams.size() < 5) {
    return cpp::fail("Invalid url: " + url);
  }

  std::string huggingFaceHost{kHuggingFaceHost};
  cortex::db::Models modellist_handler;
  std::string unique_model_id = "";
  if (temp_model_id.has_value()) {
    unique_model_id = temp_model_id.value();
  } else {
    unique_model_id = author + ":" + model_id + ":" + file_name;
  }

  auto model_entry = modellist_handler.GetModelInfo(unique_model_id);
  if (model_entry.has_value()) {
    CLI_LOG("Model already downloaded: " << unique_model_id);
    return cpp::fail("Please delete the model before downloading again");
  }

  auto local_path{file_manager_utils::GetModelsContainerPath() /
                  kHuggingFaceHost / author / model_id / file_name};

  try {
    std::filesystem::create_directories(local_path.parent_path());
  } catch (const std::filesystem::filesystem_error& e) {
    // if file exist, remove it
    std::filesystem::remove(local_path.parent_path());
    std::filesystem::create_directories(local_path.parent_path());
  }

  auto download_url = url_parser::FromUrl(url_obj.value());
  // this assume that the model being downloaded is a single gguf file
  auto downloadTask{DownloadTask{.id = model_id,
                                 .type = DownloadType::Model,
                                 .items = {DownloadItem{
                                     .id = unique_model_id,
                                     .downloadUrl = download_url,
                                     .localPath = local_path,
                                 }}}};

  auto on_finished = [author, temp_name](const DownloadTask& finishedTask) {
    // Sum downloadedBytes from all items
    uint64_t model_size = 0;
    for (const auto& item : finishedTask.items) {
      model_size = model_size + item.bytes.value_or(0);
    }
    auto gguf_download_item = finishedTask.items[0];
    ParseGguf(gguf_download_item, author, temp_name, model_size);
  };

  downloadTask.id = unique_model_id;
  return download_service_->AddTask(downloadTask, on_finished);
}

cpp::result<std::string, std::string> ModelService::HandleUrl(
    const std::string& url) {
  auto url_obj = url_parser::FromUrlString(url);
  if (url_obj.has_error()) {
    return cpp::fail("Invalid url: " + url);
  }

  if (url_obj->host == kHuggingFaceHost) {
    if (url_obj->pathParams[2] == "blob") {
      url_obj->pathParams[2] = "resolve";
    }
  }
  auto author{url_obj->pathParams[0]};
  auto model_id{url_obj->pathParams[1]};
  auto file_name{url_obj->pathParams.back()};

  if (author == "cortexso") {
    return DownloadModelFromCortexso(model_id);
  }

  if (url_obj->pathParams.size() < 5) {
    if (url_obj->pathParams.size() < 2) {
      return cpp::fail("Invalid url: " + url);
    }
    return DownloadHuggingFaceGgufModel(author, model_id, std::nullopt);
  }

  std::string huggingFaceHost{kHuggingFaceHost};
  std::string unique_model_id{author + ":" + model_id + ":" + file_name};

  cortex::db::Models modellist_handler;
  auto model_entry = modellist_handler.GetModelInfo(unique_model_id);

  if (model_entry.has_value()) {
    CLI_LOG("Model already downloaded: " << unique_model_id);
    return unique_model_id;
  }

  auto local_path{file_manager_utils::GetModelsContainerPath() /
                  kHuggingFaceHost / author / model_id / file_name};

  try {
    std::filesystem::create_directories(local_path.parent_path());
  } catch (const std::filesystem::filesystem_error& e) {
    // if file exist, remove it
    std::filesystem::remove(local_path.parent_path());
    std::filesystem::create_directories(local_path.parent_path());
  }

  auto download_url = url_parser::FromUrl(url_obj.value());
  // this assume that the model being downloaded is a single gguf file
  auto downloadTask{DownloadTask{.id = model_id,
                                 .type = DownloadType::Model,
                                 .items = {DownloadItem{
                                     .id = unique_model_id,
                                     .downloadUrl = download_url,
                                     .localPath = local_path,
                                 }}}};

  auto on_finished = [author](const DownloadTask& finishedTask) {
    // Sum downloadedBytes from all items
    uint64_t model_size = 0;
    for (const auto& item : finishedTask.items) {
      model_size = model_size + item.bytes.value_or(0);
    }
    auto gguf_download_item = finishedTask.items[0];
    ParseGguf(gguf_download_item, author, std::nullopt, model_size);
  };

  auto result = download_service_->AddDownloadTask(downloadTask, on_finished);
  if (result.has_error()) {
    CTL_ERR(result.error());
    return cpp::fail(result.error());
  } else if (result && result.value()) {
    CLI_LOG("Model " << model_id << " downloaded successfully!")
  }
  return unique_model_id;
}

bool ModelService::HasModel(const std::string& id) const {
  return cortex::db::Models().HasModel(id);
}

cpp::result<DownloadTask, std::string>
ModelService::DownloadModelFromCortexsoAsync(
    const std::string& name, const std::string& branch,
    std::optional<std::string> temp_model_id) {

  auto download_task = GetDownloadTask(name, branch);
  if (download_task.has_error()) {
    return cpp::fail(download_task.error());
  }

  cortex::db::Models modellist_handler;
  std::string unique_model_id = "";
  if (temp_model_id.has_value()) {
    unique_model_id = temp_model_id.value();
  } else {
    unique_model_id = name + ":" + branch;
  }

  auto model_entry = modellist_handler.GetModelInfo(unique_model_id);
  if (model_entry.has_value()) {
    return cpp::fail("Please delete the model before downloading again");
  }

  auto on_finished = [unique_model_id,
                      branch](const DownloadTask& finishedTask) {
    const DownloadItem* model_yml_item = nullptr;
    auto need_parse_gguf = true;

    for (const auto& item : finishedTask.items) {
      if (item.localPath.filename().string() == "model.yml") {
        model_yml_item = &item;
      }
    }

    if (model_yml_item == nullptr) {
      CTL_WRN("model.yml not found in the downloaded files for " +
              unique_model_id);
      return;
    }
    auto url_obj = url_parser::FromUrlString(model_yml_item->downloadUrl);
    CTL_INF("Adding model to modellist with branch: "
            << branch << ", path: " << model_yml_item->localPath.string());
    config::YamlHandler yaml_handler;
    yaml_handler.ModelConfigFromFile(model_yml_item->localPath.string());
    auto mc = yaml_handler.GetModelConfig();
    mc.model = unique_model_id;

    uint64_t model_size = 0;
    for (const auto& item : finishedTask.items) {
      model_size = model_size + item.bytes.value_or(0);
    }
    mc.size = model_size;
    yaml_handler.UpdateModelConfig(mc);
    yaml_handler.WriteYamlFile(model_yml_item->localPath.string());

    auto rel =
        file_manager_utils::ToRelativeCortexDataPath(model_yml_item->localPath);
    CTL_INF("path_to_model_yaml: " << rel.string());

    cortex::db::Models modellist_utils_obj;
    cortex::db::ModelEntry model_entry{.model = unique_model_id,
                                       .author_repo_id = "cortexso",
                                       .branch_name = branch,
                                       .path_to_model_yaml = rel.string(),
                                       .model_alias = unique_model_id};
    auto result = modellist_utils_obj.AddModelEntry(model_entry);
    if (result.has_error()) {
      CTL_ERR("Error adding model to modellist: " + result.error());
    }
  };

  auto task = download_task.value();
  task.id = unique_model_id;
  return download_service_->AddTask(task, on_finished);
}

cpp::result<std::string, std::string> ModelService::DownloadModelFromCortexso(
    const std::string& name, const std::string& branch) {

  auto download_task = GetDownloadTask(name, branch);
  if (download_task.has_error()) {
    return cpp::fail(download_task.error());
  }

  std::string model_id{name + ":" + branch};
  auto on_finished = [branch, model_id](const DownloadTask& finishedTask) {
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
    mc.model = model_id;
    yaml_handler.UpdateModelConfig(mc);
    yaml_handler.WriteYamlFile(model_yml_item->localPath.string());

    auto rel =
        file_manager_utils::ToRelativeCortexDataPath(model_yml_item->localPath);
    CTL_INF("path_to_model_yaml: " << rel.string());

    cortex::db::Models modellist_utils_obj;
    cortex::db::ModelEntry model_entry{.model = model_id,
                                       .author_repo_id = "cortexso",
                                       .branch_name = branch,
                                       .path_to_model_yaml = rel.string(),
                                       .model_alias = model_id};
    auto result = modellist_utils_obj.AddModelEntry(model_entry);
    if (result.has_error()) {
      CTL_ERR("Error adding model to modellist: " + result.error());
    }
  };

  auto result =
      download_service_->AddDownloadTask(download_task.value(), on_finished);
  if (result.has_error()) {
    return cpp::fail(result.error());
  } else if (result && result.value()) {
    CLI_LOG("Model " << model_id << " downloaded successfully!")
    return model_id;
  }
  return cpp::fail("Failed to download model " + model_id);
}

cpp::result<std::string, std::string>
ModelService::DownloadHuggingFaceGgufModel(
    const std::string& author, const std::string& modelName,
    std::optional<std::string> fileName) {
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
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;

  auto result = StopModel(model_handle);
  if (result.has_error()) {
    CTL_INF("Failed to stop model " << model_handle
                                    << ", error: " << result.error());
  } else {
    CTL_INF("Model " << model_handle << " stopped successfully");
  }

  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    if (model_entry.has_error()) {
      CTL_WRN("Error: " + model_entry.error());
      return cpp::fail(model_entry.error());
    }
    auto yaml_fp = fmu::ToAbsoluteCortexDataPath(
        fs::path(model_entry.value().path_to_model_yaml));
    yaml_handler.ModelConfigFromFile(yaml_fp.string());
    auto mc = yaml_handler.GetModelConfig();
    // Remove yaml file
    std::filesystem::remove(yaml_fp);
    // Remove model files if they are not imported locally
    if (model_entry.value().branch_name != "imported") {
      if (mc.files.size() > 0) {
        if (mc.engine == kLlamaRepo || mc.engine == kLlamaEngine) {
          for (auto& file : mc.files) {
            std::filesystem::path gguf_p(
                fmu::ToAbsoluteCortexDataPath(fs::path(file)));
            std::filesystem::remove(gguf_p);
          }
        } else {
          std::filesystem::path f(
              fmu::ToAbsoluteCortexDataPath(fs::path(mc.files[0])));
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

cpp::result<StartModelResult, std::string> ModelService::StartModel(
    const std::string& model_handle,
    const StartParameterOverride& params_override) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;

  try {
    Json::Value json_data;
    // Currently we don't support download vision models, so we need to bypass check
    if (!params_override.bypass_model_check()) {
      auto model_entry = modellist_handler.GetModelInfo(model_handle);
      if (model_entry.has_error()) {
        CTL_WRN("Error: " + model_entry.error());
        return cpp::fail(model_entry.error());
      }
      yaml_handler.ModelConfigFromFile(
          fmu::ToAbsoluteCortexDataPath(
              fs::path(model_entry.value().path_to_model_yaml))
              .string());
      auto mc = yaml_handler.GetModelConfig();

      json_data = mc.ToJson();
      if (mc.files.size() > 0) {
#if defined(_WIN32)
        json_data["model_path"] = cortex_utils::WstringToUtf8(
            fmu::ToAbsoluteCortexDataPath(fs::path(mc.files[0])).wstring());
#else
        json_data["model_path"] =
            fmu::ToAbsoluteCortexDataPath(fs::path(mc.files[0])).string();
#endif
      } else {
        LOG_WARN << "model_path is empty";
        return StartModelResult{.success = false};
      }
      json_data["system_prompt"] = mc.system_template;
      json_data["user_prompt"] = mc.user_template;
      json_data["ai_prompt"] = mc.ai_template;
    } else {
      bypass_stop_check_set_.insert(model_handle);
    }

    json_data["model"] = model_handle;
    if (auto& cpt = params_override.custom_prompt_template;
        !cpt.value_or("").empty()) {
      auto parse_prompt_result = string_utils::ParsePrompt(cpt.value());
      json_data["system_prompt"] = parse_prompt_result.system_prompt;
      json_data["user_prompt"] = parse_prompt_result.user_prompt;
      json_data["ai_prompt"] = parse_prompt_result.ai_prompt;
    }

#define ASSIGN_IF_PRESENT(json_obj, param_override, param_name) \
  if (param_override.param_name) {                              \
    json_obj[#param_name] = param_override.param_name.value();  \
  }

    ASSIGN_IF_PRESENT(json_data, params_override, cache_enabled);
    ASSIGN_IF_PRESENT(json_data, params_override, ngl);
    ASSIGN_IF_PRESENT(json_data, params_override, n_parallel);
    ASSIGN_IF_PRESENT(json_data, params_override, ctx_len);
    ASSIGN_IF_PRESENT(json_data, params_override, cache_type);
    ASSIGN_IF_PRESENT(json_data, params_override, mmproj);
    ASSIGN_IF_PRESENT(json_data, params_override, model_path);
#undef ASSIGN_IF_PRESENT

    CTL_INF(json_data.toStyledString());
    // TODO(sang) move this into another function
    // Calculate ram/vram needed to load model
    services::HardwareService hw_svc;
    auto hw_info = hw_svc.GetHardwareInfo();
    assert(!!engine_svc_);
    auto default_engine = engine_svc_->GetDefaultEngineVariant(kLlamaEngine);
    bool is_cuda = false;
    if (default_engine.has_error()) {
      CTL_INF("Could not get default engine");
    } else {
      auto& de = default_engine.value();
      is_cuda = de.variant.find("cuda") != std::string::npos;
      CTL_INF("is_cuda: " << is_cuda);
    }

    std::optional<std::string> warning;
    if (is_cuda && !system_info_utils::IsNvidiaSmiAvailable()) {
      CTL_INF(
          "Running cuda variant but nvidia-driver is not installed yet, "
          "fallback to CPU mode");
      auto res = engine_svc_->GetInstalledEngineVariants(kLlamaEngine);
      if (res.has_error()) {
        CTL_WRN("Could not get engine variants");
        return cpp::fail("Nvidia-driver is not installed!");
      } else {
        auto& es = res.value();
        std::sort(
            es.begin(), es.end(),
            [](const EngineVariantResponse& e1,
               const EngineVariantResponse& e2) { return e1.name > e2.name; });
        for (auto& e : es) {
          CTL_INF(e.name << " " << e.version << " " << e.engine);
          // Select the first CPU candidate
          if (e.name.find("cuda") == std::string::npos) {
            auto r = engine_svc_->SetDefaultEngineVariant(kLlamaEngine,
                                                          e.version, e.name);
            if (r.has_error()) {
              CTL_WRN("Could not set default engine variant");
              return cpp::fail("Nvidia-driver is not installed!");
            } else {
              CTL_INF("Change default engine to: " << e.name);
              auto rl = engine_svc_->LoadEngine(kLlamaEngine);
              if (rl.has_error()) {
                return cpp::fail("Nvidia-driver is not installed!");
              } else {
                CTL_INF("Engine started");
                is_cuda = false;
                warning = "Nvidia-driver is not installed, use CPU variant: " +
                          e.version + "-" + e.name;
                break;
              }
            }
          }
        }
        // If we reach here, means that no CPU variant to fallback
        if (!warning) {
          return cpp::fail(
              "Nvidia-driver is not installed, no available CPU version to "
              "fallback");
        }
      }
    }
    // If in GPU acceleration mode:
    // We use all visible GPUs, so only need to sum all free vram
    auto free_vram_MiB = 0u;
    for (const auto& gpu : hw_info.gpus) {
      free_vram_MiB += gpu.free_vram;
    }

    auto free_ram_MiB = hw_info.ram.available_MiB;

    auto const& mp = json_data["model_path"].asString();
    auto ngl = json_data["ngl"].asInt();
    // Bypass for now
    auto vram_needed_MiB = 0u;
    auto ram_needed_MiB = 0u;

    if (vram_needed_MiB > free_vram_MiB && is_cuda) {
      CTL_WRN("Not enough VRAM - " << "required: " << vram_needed_MiB
                                   << ", available: " << free_vram_MiB);

      return cpp::fail(
          "Not enough VRAM - required: " + std::to_string(vram_needed_MiB) +
          " MiB, available: " + std::to_string(free_vram_MiB) +
          " MiB - Should adjust ngl to " +
          std::to_string(free_vram_MiB / (vram_needed_MiB / ngl) - 1));
    }

    if (ram_needed_MiB > free_ram_MiB) {
      CTL_WRN("Not enough RAM - " << "required: " << ram_needed_MiB
                                  << ", available: " << free_ram_MiB);
      return cpp::fail(
          "Not enough RAM - required: " + std::to_string(ram_needed_MiB) +
          " MiB,, available: " + std::to_string(free_ram_MiB) + " MiB");
    }

    assert(!!inference_svc_);
    auto ir =
        inference_svc_->LoadModel(std::make_shared<Json::Value>(json_data));
    auto status = std::get<0>(ir)["status_code"].asInt();
    auto data = std::get<1>(ir);
    if (status == httplib::StatusCode::OK_200) {
      return StartModelResult{.success = true, .warning = warning};
    } else if (status == httplib::StatusCode::Conflict_409) {
      CTL_INF("Model '" + model_handle + "' is already loaded");
      return StartModelResult{.success = true, .warning = warning};
    } else {
      // only report to user the error
      CTL_ERR("Model failed to start with status code: " << status);
      return cpp::fail("Model failed to start: " + data["message"].asString());
    }
  } catch (const std::exception& e) {
    return cpp::fail("Fail to load model with ID '" + model_handle +
                     "': " + e.what());
  }
}

cpp::result<bool, std::string> ModelService::StopModel(
    const std::string& model_handle) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;

  try {
    auto bypass_check = (bypass_stop_check_set_.find(model_handle) !=
                         bypass_stop_check_set_.end());
    std::string engine_name = "";
    if (!bypass_check) {
      auto model_entry = modellist_handler.GetModelInfo(model_handle);
      if (model_entry.has_error()) {
        CTL_WRN("Error: " + model_entry.error());
        return cpp::fail(model_entry.error());
      }
      yaml_handler.ModelConfigFromFile(
          fmu::ToAbsoluteCortexDataPath(
              fs::path(model_entry.value().path_to_model_yaml))
              .string());
      auto mc = yaml_handler.GetModelConfig();
      engine_name = mc.engine;
    }
    if (bypass_check) {
      engine_name = kLlamaEngine;
    }
    assert(inference_svc_);
    auto ir = inference_svc_->UnloadModel(engine_name, model_handle);
    auto status = std::get<0>(ir)["status_code"].asInt();
    auto data = std::get<1>(ir);
    if (status == httplib::StatusCode::OK_200) {
      if (bypass_check) {
        bypass_stop_check_set_.erase(model_handle);
      }
      return true;
    } else {
      CTL_ERR("Model failed to stop with status code: " << status);
      return cpp::fail("Model failed to stop: " + data["message"].asString());
    }
  } catch (const std::exception& e) {
    return cpp::fail("Fail to unload model with ID '" + model_handle +
                     "': " + e.what());
  }
}

cpp::result<bool, std::string> ModelService::GetModelStatus(
    const std::string& model_handle) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;

  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    if (model_entry.has_error()) {
      CTL_WRN("Error: " + model_entry.error());
      return cpp::fail(model_entry.error());
    }
    yaml_handler.ModelConfigFromFile(
        fmu::ToAbsoluteCortexDataPath(
            fs::path(model_entry.value().path_to_model_yaml))
            .string());
    auto mc = yaml_handler.GetModelConfig();

    Json::Value root;
    root["model"] = model_handle;
    root["engine"] = mc.engine;

    auto ir =
        inference_svc_->GetModelStatus(std::make_shared<Json::Value>(root));
    auto status = std::get<0>(ir)["status_code"].asInt();
    auto data = std::get<1>(ir);
    if (status == httplib::StatusCode::OK_200) {
      return true;
    } else {
      CTL_ERR("Model failed to get model status with status code: " << status);
      return cpp::fail("Model failed to get model status: " +
                       data["message"].asString());
    }
  } catch (const std::exception& e) {
    return cpp::fail("Fail to get model status with ID '" + model_handle +
                     "': " + e.what());
  }
}

cpp::result<ModelPullInfo, std::string> ModelService::GetModelPullInfo(
    const std::string& input) {
  if (input.empty()) {
    return cpp::fail(
        "Input must be Cortex Model Hub handle or HuggingFace url!");
  }
  auto model_name = input;

  if (string_utils::StartsWith(input, "https://")) {
    auto url_obj = url_parser::FromUrlString(input);
    if (url_obj.has_error()) {
      return cpp::fail("Invalid url: " + input);
    }
    if (url_obj->host == kHuggingFaceHost) {
      if (url_obj->pathParams[2] == "blob") {
        url_obj->pathParams[2] = "resolve";
      }
    }

    auto author{url_obj->pathParams[0]};
    auto model_id{url_obj->pathParams[1]};
    auto file_name{url_obj->pathParams.back()};
    if (author == "cortexso") {
      return ModelPullInfo{
          .id = model_id + ":" + url_obj->pathParams[3],
          .downloaded_models = {},
          .available_models = {},
          .download_url = url_parser::FromUrl(url_obj.value())};
    }
    return ModelPullInfo{.id = author + ":" + model_id + ":" + file_name,
                         .downloaded_models = {},
                         .available_models = {},
                         .download_url = url_parser::FromUrl(url_obj.value())};
  }

  if (input.find(":") != std::string::npos) {
    auto parsed = string_utils::SplitBy(input, ":");
    if (parsed.size() != 2) {
      return cpp::fail("Invalid model handle: " + input);
    }
    return ModelPullInfo{.id = input,
                         .downloaded_models = {},
                         .available_models = {},
                         .download_url = input};
  }

  if (input.find("/") != std::string::npos) {
    auto parsed = string_utils::SplitBy(input, "/");
    if (parsed.size() != 2) {
      return cpp::fail("Invalid model handle: " + input);
    }

    auto author = parsed[0];
    model_name = parsed[1];
    if (author != "cortexso") {
      auto repo_info =
          huggingface_utils::GetHuggingFaceModelRepoInfo(author, model_name);

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

      return ModelPullInfo{
          .id = author + ":" + model_name,
          .downloaded_models = {},
          .available_models = options,
          .download_url =
              huggingface_utils::GetDownloadableUrl(author, model_name, "")};
    }
  }
  auto branches =
      huggingface_utils::GetModelRepositoryBranches("cortexso", model_name);
  if (branches.has_error()) {
    return cpp::fail(branches.error());
  }

  auto default_model_branch = huggingface_utils::GetDefaultBranch(model_name);

  cortex::db::Models modellist_handler;
  auto downloaded_model_ids = modellist_handler.FindRelatedModel(model_name)
                                  .value_or(std::vector<std::string>{});

  std::vector<std::string> avai_download_opts{};
  for (const auto& branch : branches.value()) {
    if (branch.second.name == "main") {  // main branch only have metadata. skip
      continue;
    }
    auto model_id = model_name + ":" + branch.second.name;
    if (std::find(downloaded_model_ids.begin(), downloaded_model_ids.end(),
                  model_id) !=
        downloaded_model_ids.end()) {  // if downloaded, we skip it
      continue;
    }
    avai_download_opts.emplace_back(model_id);
  }

  if (avai_download_opts.empty()) {
    // TODO: only with pull, we return
    return cpp::fail("No variant available");
  }
  std::optional<std::string> normalized_def_branch = std::nullopt;
  if (default_model_branch.has_value()) {
    normalized_def_branch = model_name + ":" + default_model_branch.value();
  }
  string_utils::SortStrings(downloaded_model_ids);
  string_utils::SortStrings(avai_download_opts);

  return ModelPullInfo{.id = model_name,
                       .default_branch = normalized_def_branch.value_or(""),
                       .downloaded_models = downloaded_model_ids,
                       .available_models = avai_download_opts,
                       .model_source = "cortexso"};
}

cpp::result<std::string, std::string> ModelService::AbortDownloadModel(
    const std::string& task_id) {
  return download_service_->StopTask(task_id);
}
