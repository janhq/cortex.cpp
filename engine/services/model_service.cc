#include "model_service.h"
#include <filesystem>
#include <iostream>
#include <optional>
#include <ostream>
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "database/models.h"
#include "httplib.h"
#include "utils/cli_selection_utils.h"
#include "utils/engine_constants.h"
#include "utils/file_manager_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "utils/string_utils.h"

namespace {
void ParseGguf(const DownloadItem& ggufDownloadItem,
               std::optional<std::string> author) {
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
  yaml_handler.UpdateModelConfig(model_config);

  auto yaml_path{ggufDownloadItem.localPath};
  auto yaml_name = yaml_path.replace_extension(".yml");

  if (!std::filesystem::exists(yaml_path)) {
    yaml_handler.WriteYamlFile(yaml_path.string());
  }

  auto url_obj = url_parser::FromUrlString(ggufDownloadItem.downloadUrl);
  auto branch = url_obj.pathParams[3];
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
  using namespace nlohmann;
  url_parser::Url url = {
      .protocol = "https",
      .host = ModelService::kHuggingFaceHost,
      .pathParams = {"api", "models", "cortexso", modelId, "tree", branch}};

  httplib::Client cli(url.GetProtocolAndHost());
  auto res =
      cli.Get(url.GetPathAndQuery(), huggingface_utils::CreateHttpHfHeaders());
  if (res->status != httplib::StatusCode::OK_200) {
    return cpp::fail("Model " + modelId + " not found");
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
    const std::string& url, std::optional<std::string> temp_model_id) {
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
    return DownloadModelFromCortexsoAsync(model_id);
  }

  if (url_obj.pathParams.size() < 5) {
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
    auto gguf_download_item = finishedTask.items[0];
    ParseGguf(gguf_download_item, author);
  };

  downloadTask.id = unique_model_id;
  return download_service_->AddTask(downloadTask, on_finished);
}

cpp::result<std::string, std::string> ModelService::HandleUrl(
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

  if (url_obj.pathParams.size() < 5) {
    if (url_obj.pathParams.size() < 2) {
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
    auto gguf_download_item = finishedTask.items[0];
    ParseGguf(gguf_download_item, author);
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
  auto on_finished = [&, unique_model_id](const DownloadTask& finishedTask) {
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
    CTL_INF("Adding model to modellist with branch: " << branch);
    config::YamlHandler yaml_handler;
    yaml_handler.ModelConfigFromFile(model_yml_item->localPath.string());
    auto mc = yaml_handler.GetModelConfig();
    mc.model = unique_model_id;
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
  auto on_finished = [&, model_id](const DownloadTask& finishedTask) {
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

cpp::result<bool, std::string> ModelService::StartModel(
    const std::string& host, int port, const std::string& model_handle,
    std::optional<std::string> custom_prompt_template) {
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

    httplib::Client cli(host + ":" + std::to_string(port));

    Json::Value json_data = mc.ToJson();
    if (mc.files.size() > 0) {
      // TODO(sang) support multiple files
      json_data["model_path"] =
          fmu::ToAbsoluteCortexDataPath(fs::path(mc.files[0])).string();
    } else {
      LOG_WARN << "model_path is empty";
      return false;
    }
    json_data["model"] = model_handle;
    if (!custom_prompt_template.value_or("").empty()) {
      auto parse_prompt_result =
          string_utils::ParsePrompt(custom_prompt_template.value());
      json_data["system_prompt"] = parse_prompt_result.system_prompt;
      json_data["user_prompt"] = parse_prompt_result.user_prompt;
      json_data["ai_prompt"] = parse_prompt_result.ai_prompt;
    } else {
      json_data["system_prompt"] = mc.system_template;
      json_data["user_prompt"] = mc.user_template;
      json_data["ai_prompt"] = mc.ai_template;
    }

    auto data_str = json_data.toStyledString();
    CTL_INF(data_str);
    cli.set_read_timeout(std::chrono::seconds(60));
    auto res = cli.Post("/inferences/server/loadmodel", httplib::Headers(),
                        data_str.data(), data_str.size(), "application/json");
    if (res) {
      if (res->status == httplib::StatusCode::OK_200) {
        return true;
      } else if (res->status == httplib::StatusCode::Conflict_409) {
        CTL_INF("Model '" + model_handle + "' is already loaded");
        return true;
      } else {
        CTL_ERR("Model failed to load with status code: " << res->status);
        return cpp::fail("Model failed to load with status code: " +
                         std::to_string(res->status));
      }
    } else {
      auto err = res.error();
      CTL_ERR("HTTP error: " << httplib::to_string(err));
      return cpp::fail("HTTP error: " + httplib::to_string(err));
    }

  } catch (const std::exception& e) {
    return cpp::fail("Fail to load model with ID '" + model_handle +
                     "': " + e.what());
  }
}

cpp::result<bool, std::string> ModelService::StopModel(
    const std::string& host, int port, const std::string& model_handle) {
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

    httplib::Client cli(host + ":" + std::to_string(port));

    Json::Value json_data;
    json_data["model"] = model_handle;
    json_data["engine"] = mc.engine;
    auto data_str = json_data.toStyledString();
    CTL_INF(data_str);
    cli.set_read_timeout(std::chrono::seconds(60));
    auto res = cli.Post("/inferences/server/unloadmodel", httplib::Headers(),
                        data_str.data(), data_str.size(), "application/json");
    if (res) {
      if (res->status == httplib::StatusCode::OK_200) {
        return true;
      } else {
        CTL_ERR("Model failed to unload with status code: " << res->status);
        return cpp::fail("Model failed to unload with status code: " +
                         std::to_string(res->status));
      }
    } else {
      auto err = res.error();
      CTL_ERR("HTTP error: " << httplib::to_string(err));
      return cpp::fail("HTTP error: " + httplib::to_string(err));
    }

  } catch (const std::exception& e) {
    return cpp::fail("Fail to unload model with ID '" + model_handle +
                     "': " + e.what());
  }
}

cpp::result<bool, std::string> ModelService::GetModelStatus(
    const std::string& host, int port, const std::string& model_handle) {
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

    httplib::Client cli(host + ":" + std::to_string(port));
    nlohmann::json json_data;
    json_data["model"] = model_handle;
    json_data["engine"] = mc.engine;

    auto data_str = json_data.dump();

    auto res = cli.Post("/inferences/server/modelstatus", httplib::Headers(),
                        data_str.data(), data_str.size(), "application/json");
    if (res) {
      if (res->status == httplib::StatusCode::OK_200) {
        return true;
      } else {
        CTL_INF("Model failed to get model status with status code: "
                << res->status);
        return cpp::fail("Model failed to get model status with status code: " +
                         std::to_string(res->status));
      }
    } else {
      auto err = res.error();
      CTL_WRN("HTTP error: " << httplib::to_string(err));
      return cpp::fail("HTTP error: " + httplib::to_string(err));
    }
  } catch (const std::exception& e) {
    return cpp::fail("Fail to get model status with ID '" + model_handle +
                     "': " + e.what());
  }
}

cpp::result<std::string, std::string> ModelService::AbortDownloadModel(
    const std::string& task_id) {
  return download_service_->StopTask(task_id);
}
