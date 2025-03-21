#include "model_service.h"
#include <curl/multi.h>
#include <drogon/HttpTypes.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "database/models.h"
#include "hardware_service.h"
#include "utils/archive_utils.h"

#include "services/inference_service.h"

#include "utils/cli_selection_utils.h"
#include "utils/engine_constants.h"
#include "utils/file_manager_utils.h"
#include "utils/gguf_metadata_reader.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "utils/set_permission_utils.h"
#include "utils/string_utils.h"
#include "utils/widechar_conv.h"

namespace {
void ParseGguf(DatabaseService& db_service,
               const DownloadItem& ggufDownloadItem,
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
  CTL_INF("path_to_model_yaml: " << rel.string()
                                 << ", model: " << ggufDownloadItem.id);

  auto author_id = author.has_value() ? author.value() : "cortexso";
  if (!db_service.HasModel(ggufDownloadItem.id)) {
    cortex::db::ModelEntry model_entry{
        /* .model = */ ggufDownloadItem.id,
        /* .author_repo_id = */ author_id,
        /* .branch_name = */ branch,
        /* .path_to_model_yaml = */ rel.string(),
        /* .model_alias = */ ggufDownloadItem.id,
        "",
        "",
        /* .status = */ cortex::db::ModelStatus::Downloaded,
        "",
        ""};
    auto result = db_service.AddModelEntry(model_entry);

    if (result.has_error()) {
      CTL_ERR("Error adding model to modellist: " + result.error());
    }
  } else {
    if (auto m = db_service.GetModelInfo(ggufDownloadItem.id); m.has_value()) {
      auto upd_m = m.value();
      upd_m.path_to_model_yaml = rel.string();
      upd_m.status = cortex::db::ModelStatus::Downloaded;
      if (auto r = db_service.UpdateModelEntry(ggufDownloadItem.id, upd_m);
          r.has_error()) {
        CTL_ERR(r.error());
      }
    }
  }
}

cpp::result<DownloadTask, std::string> GetCloneRepoDownloadTask(
    const std::string& author_id, const std::string& modelId,
    const std::string& branch, const std::vector<std::string>& save_dir,
    const std::string& task_id) {
  url_parser::Url url = {/* .protocol = */ "https",
                         /* .host = */ kHuggingFaceHost,
                         /* .pathParams = */
                         {"api", "models", author_id, modelId, "tree", branch},
                         {}};

  auto result = curl_utils::SimpleGetJsonRecursive(url.ToFullPath());
  if (result.has_error()) {
    return cpp::fail("Model " + modelId + " not found");
  }

  std::vector<DownloadItem> download_items{};
  auto model_container_path = file_manager_utils::GetModelsContainerPath();
  for (auto subdir : save_dir)
    model_container_path /= subdir;
  file_manager_utils::CreateDirectoryRecursively(model_container_path.string());

  for (const auto& value : result.value()) {
    // std::cout << "value object: " << value.toStyledString() << std::endl;
    auto path = value["path"].asString();
    if (path == ".gitattributes" || path == ".gitignore" ||
        path == "README.md") {
      continue;
    }
    url_parser::Url download_url = {
        /* .protocol = */ "https",
        /* .host = */ kHuggingFaceHost,
        /* .pathParams = */ {author_id, modelId, "resolve", branch, path},
        {}};

    auto local_path = model_container_path / path;
    if (!std::filesystem::exists(local_path.parent_path())) {
      std::filesystem::create_directories(local_path.parent_path());
    }
    download_items.push_back(DownloadItem{
        /* .id = */ path,
        /* .downloadUrl = */ download_url.ToFullPath(),
        /* .localPath = */ local_path,
        /*.checksum = */ std::nullopt,
        /* .bytes = */ std::nullopt,
        /* .downloadedBytes = */ std::nullopt,
    });
  }

  return DownloadTask{
      /* .id = */ task_id,
      /* .status = */ DownloadTask::Status::Pending,
      /* .type = */ DownloadType::Model,
      /* .items = */ download_items};
}
}  // namespace

ModelService::ModelService(std::shared_ptr<DatabaseService> db_service,
                           std::shared_ptr<HardwareService> hw_service,
                           std::shared_ptr<DownloadService> download_service,
                           std::shared_ptr<InferenceService> inference_service,
                           std::shared_ptr<EngineServiceI> engine_svc,
                           cortex::TaskQueue& task_queue)
    : db_service_(db_service),
      hw_service_(hw_service),
      download_service_{download_service},
      inference_svc_(inference_service),
      engine_svc_(engine_svc),
      task_queue_(task_queue) {
        // ProcessBgrTasks();
      };

void ModelService::ForceIndexingModelList() {
  CTL_INF("Force indexing model list");

  config::YamlHandler yaml_handler;

  auto list_entry = db_service_->LoadModelList();
  if (list_entry.has_error()) {
    CTL_ERR("Failed to load model list: " << list_entry.error());
    return;
  }

  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;

  CTL_DBG("Database model size: " + std::to_string(list_entry.value().size()));
  for (const auto& model_entry : list_entry.value()) {
    if (model_entry.status != cortex::db::ModelStatus::Downloaded) {
      continue;
    }
    if (model_entry.engine == kVllmEngine) {
      // TODO: check if folder still exists?
      continue;
    }
    try {
      // check if path_to_model_yaml still exists
      CTL_DBG(fmu::ToAbsoluteCortexDataPath(
                  fs::path(model_entry.path_to_model_yaml))
                  .string());
      yaml_handler.ModelConfigFromFile(
          fmu::ToAbsoluteCortexDataPath(
              fs::path(model_entry.path_to_model_yaml))
              .string());
      auto model_config = yaml_handler.GetModelConfig();
      Json::Value obj = model_config.ToJson();
      yaml_handler.Reset();
    } catch (const std::exception& e) {
      // remove in db
      auto remove_result = db_service_->DeleteModelEntry(model_entry.model);
      CTL_DBG(e.what());
      // silently ignore result
    }
  }
}

std::optional<config::ModelConfig> ModelService::GetDownloadedModel(
    const std::string& modelId) const {

  config::YamlHandler yaml_handler;
  auto result = db_service_->GetModelInfo(modelId);
  if (result.has_error()) {
    return std::nullopt;
  }
  auto model_entry = result.value();

  // ignore all other params
  if (model_entry.engine == kVllmEngine) {
    config::ModelConfig cfg;
    cfg.engine = kVllmEngine;
    return cfg;
  }

  try {
    config::YamlHandler yaml_handler;
    namespace fs = std::filesystem;
    namespace fmu = file_manager_utils;
    yaml_handler.ModelConfigFromFile(
        fmu::ToAbsoluteCortexDataPath(fs::path(model_entry.path_to_model_yaml))
            .string());
    return yaml_handler.GetModelConfig();
  } catch (const std::exception& e) {
    LOG_ERROR << "Error reading yaml file '" << model_entry.path_to_model_yaml
              << "': " << e.what();
    return std::nullopt;
  }
}

cpp::result<DownloadTask, std::string> ModelService::HandleDownloadUrlAsync(
    const std::string& url, std::optional<std::string> temp_model_id,
    std::optional<std::string> temp_name) {
  auto url_obj = url_parser::FromUrlString(url);
  if (url_obj.has_error() || url_obj->pathParams.size() < 5) {
    return cpp::fail(
        "Invalid url: " + url +
        ", a valid URL example is: "
        "https://huggingface.co/cortexso/tinyllama/blob/1b/model.gguf");
  }

  if (url_obj->host == kHuggingFaceHost) {
    if (url_obj->pathParams[2] == "blob") {
      url_obj->pathParams[2] = "resolve";
    }
  } else {
    return cpp::fail("Only support pull model from " +
                     std::string(kHuggingFaceHost));
  }
  auto author{url_obj->pathParams[0]};
  auto model_id{url_obj->pathParams[1]};
  auto file_name{url_obj->pathParams.back()};

  if (author == "cortexso") {
    return DownloadModelFromCortexsoAsync(model_id, url_obj->pathParams[3]);
  }

  std::string huggingFaceHost{kHuggingFaceHost};
  std::string unique_model_id = "";
  if (temp_model_id.has_value()) {
    unique_model_id = temp_model_id.value();
  } else {
    unique_model_id = author + ":" + model_id + ":" + file_name;
  }

  auto model_entry = db_service_->GetModelInfo(unique_model_id);
  if (model_entry.has_value() &&
      model_entry->status == cortex::db::ModelStatus::Downloaded) {
    CLI_LOG("Model already downloaded: " << unique_model_id);
    return cpp::fail("Please delete the model before downloading again");
  }

  auto local_path{file_manager_utils::GetModelsContainerPath() /
                  kHuggingFaceHost / author / model_id / file_name};

  try {
    std::filesystem::create_directories(local_path.parent_path());
  } catch (const std::filesystem::filesystem_error&) {
    // if file exist, remove it
    std::filesystem::remove(local_path.parent_path());
    std::filesystem::create_directories(local_path.parent_path());
  }

  auto download_url = url_parser::FromUrl(url_obj.value());
  // this assume that the model being downloaded is a single gguf file
  auto downloadTask{DownloadTask{/* .id = */ model_id,
                                 DownloadTask::Status::Pending,
                                 /* .type = */ DownloadType::Model,
                                 /* .items = */
                                 {DownloadItem{
                                     /* .id = */ unique_model_id,
                                     /* .downloadUrl = */ download_url,
                                     /* .localPath = */ local_path,
                                     /* .checksum = */ std::nullopt,
                                     /* .bytes = */ std::nullopt,
                                     /* .downloadedBytes = */ std::nullopt,
                                 }}}};

  auto on_finished = [this, author,
                      temp_name](const DownloadTask& finishedTask) {
    // Sum downloadedBytes from all items
    uint64_t model_size = 0;
    for (const auto& item : finishedTask.items) {
      model_size = model_size + item.bytes.value_or(0);
    }
    auto gguf_download_item = finishedTask.items[0];
    ParseGguf(*db_service_, gguf_download_item, author, temp_name, model_size);
  };

  downloadTask.id = unique_model_id;
  return download_service_->AddTask(downloadTask, on_finished);
}

cpp::result<DownloadTask, std::string> ModelService::DownloadHfModelAsync(
    const std::string& author_id, const std::string& model_id) {

  const std::string unique_model_id = author_id + "/" + model_id;
  auto model_entry = db_service_->GetModelInfo(unique_model_id);
  if (model_entry.has_value() &&
      model_entry->status == cortex::db::ModelStatus::Downloaded)
    return cpp::fail("Please delete the model before downloading again");

  auto download_task = GetCloneRepoDownloadTask(
      author_id, model_id, "main", {kHuggingFaceHost, author_id, model_id},
      unique_model_id);
  if (download_task.has_error())
    return download_task;

  // TODO: validate that this is a vllm-compatible model
  auto on_finished = [this, author_id,
                      unique_model_id](const DownloadTask& finishedTask) {
    if (!db_service_->HasModel(unique_model_id)) {
      CTL_INF("Before creating model entry");
      cortex::db::ModelEntry model_entry{
          .model = unique_model_id,
          .author_repo_id = author_id,
          .branch_name = "main",
          .path_to_model_yaml = "",
          .model_alias = unique_model_id,
          .status = cortex::db::ModelStatus::Downloaded,
          .engine = kVllmEngine};

      auto result = db_service_->AddModelEntry(model_entry);
      if (result.has_error()) {
        CTL_ERR("Error adding model to modellist: " + result.error());
      }
    } else {
      if (auto m = db_service_->GetModelInfo(unique_model_id); m.has_value()) {
        auto upd_m = m.value();
        upd_m.status = cortex::db::ModelStatus::Downloaded;
        if (auto r = db_service_->UpdateModelEntry(unique_model_id, upd_m);
            r.has_error()) {
          CTL_ERR(r.error());
        }
      } else {
        CTL_WRN("Could not get model entry with model id: " << unique_model_id);
      }
    }
  };

  return download_service_->AddTask(download_task.value(), on_finished);
}

std::optional<hardware::Estimation> ModelService::GetEstimation(
    const std::string& model_handle) {
  std::lock_guard l(es_mtx_);
  if (auto it = es_.find(model_handle); it != es_.end()) {
    return it->second;
  }
  return std::nullopt;
}

cpp::result<std::optional<hardware::Estimation>, std::string>
ModelService::EstimateModel(const std::string& model_handle,
                            const std::string& kv_cache, int n_batch,
                            int n_ubatch) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  config::YamlHandler yaml_handler;

  try {
    auto model_entry = db_service_->GetModelInfo(model_handle);
    if (model_entry.has_error()) {
      CTL_WRN("Error: " + model_entry.error());
      return cpp::fail(model_entry.error());
    }
    yaml_handler.ModelConfigFromFile(
        fmu::ToAbsoluteCortexDataPath(
            fs::path(model_entry.value().path_to_model_yaml))
            .string());
    auto mc = yaml_handler.GetModelConfig();
    assert(hw_service_);
    auto hw_info = hw_service_->GetHardwareInfo();
    int64_t free_vram_MiB = 0;
    for (const auto& gpu : hw_info.gpus) {
      free_vram_MiB += gpu.free_vram;
    }

#if defined(__APPLE__) && defined(__MACH__)
    free_vram_MiB = hw_info.ram.available_MiB;
#endif

    return hardware::EstimateLLaMACppRun(
        fmu::ToAbsoluteCortexDataPath(fs::path(mc.files[0])).string(),
        {/* .ngl = */ mc.ngl,
         /* .ctx_len = */ mc.ctx_len,
         /* .n_batch = */ n_batch,
         /* .n_ubatch = */ n_ubatch,
         /* .kv_cache_type = */ kv_cache,
         /* .free_vram_MiB = */ free_vram_MiB});
  } catch (const std::exception& e) {
    return cpp::fail("Fail to get model status with ID '" + model_handle +
                     "': " + e.what());
  }
}

bool ModelService::HasModel(const std::string& id) const {
  return db_service_->HasModel(id);
}

cpp::result<DownloadTask, std::string>
ModelService::DownloadModelFromCortexsoAsync(
    const std::string& model_name, const std::string& branch,
    std::optional<std::string> temp_model_id) {

  std::string unique_model_id =
      temp_model_id.value_or(model_name + ":" + branch);
  auto model_entry = db_service_->GetModelInfo(unique_model_id);
  if (model_entry.has_value() &&
      model_entry->status == cortex::db::ModelStatus::Downloaded) {
    return cpp::fail("Please delete the model before downloading again");
  }

  auto download_task = GetCloneRepoDownloadTask(
      "cortexso", model_name, branch, {"cortex.so", model_name, branch},
      unique_model_id);
  if (download_task.has_error()) {
    return cpp::fail(download_task.error());
  }

  auto on_finished = [this, unique_model_id,
                      branch](const DownloadTask& finishedTask) {
    const DownloadItem* model_yml_item = nullptr;
    // [unused] auto need_parse_gguf = true;

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

    if (mc.engine == kLlamaEngine) {
      mc.model = unique_model_id;

      uint64_t model_size = 0;
      for (const auto& item : finishedTask.items) {
        model_size = model_size + item.bytes.value_or(0);
      }
      mc.size = model_size;
      yaml_handler.UpdateModelConfig(mc);
      yaml_handler.WriteYamlFile(model_yml_item->localPath.string());
    }

    auto rel =
        file_manager_utils::ToRelativeCortexDataPath(model_yml_item->localPath);
    CTL_INF("path_to_model_yaml: " << rel.string());

    if (!db_service_->HasModel(unique_model_id)) {
      cortex::db::ModelEntry model_entry{
          /* .model = */ unique_model_id,
          /* .author_repo_id = */ "cortexso",
          /* .branch_name = */ branch,
          /* .path_to_model_yaml = */ rel.string(),
          /* .model_alias = */ unique_model_id,
          "",
          "",
          /* .status = */ cortex::db::ModelStatus::Downloaded,
          /* .engine = */ mc.engine,
          ""};
      auto result = db_service_->AddModelEntry(model_entry);

      if (result.has_error()) {
        CTL_ERR("Error adding model to modellist: " + result.error());
      }
    } else {
      if (auto m = db_service_->GetModelInfo(unique_model_id); m.has_value()) {
        auto upd_m = m.value();
        upd_m.path_to_model_yaml = rel.string();
        upd_m.status = cortex::db::ModelStatus::Downloaded;
        if (auto r = db_service_->UpdateModelEntry(unique_model_id, upd_m);
            r.has_error()) {
          CTL_ERR(r.error());
        }
      } else {
        CTL_WRN("Could not get model entry with model id: " << unique_model_id);
      }
    }
  };

  return download_service_->AddTask(download_task.value(), on_finished);
}

cpp::result<void, std::string> ModelService::DeleteModel(
    const std::string& model_handle) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  config::YamlHandler yaml_handler;

  auto result = StopModel(model_handle);
  if (result.has_error()) {
    CTL_INF("Failed to stop model " << model_handle
                                    << ", error: " << result.error());
  } else {
    CTL_INF("Model " << model_handle << " stopped successfully");
  }

  try {
    auto model_entry = db_service_->GetModelInfo(model_handle);
    if (model_entry.has_error()) {
      CTL_WRN("Error: " + model_entry.error());
      return cpp::fail(model_entry.error());
    }
    auto yaml_fp = fmu::ToAbsoluteCortexDataPath(
        fs::path(model_entry.value().path_to_model_yaml));
    yaml_handler.ModelConfigFromFile(yaml_fp.string());
    auto mc = yaml_handler.GetModelConfig();
    if (engine_svc_->IsRemoteEngine(mc.engine)) {
      std::filesystem::remove(yaml_fp);
      CTL_INF("Removed: " << yaml_fp.string());
    } else {
      // Remove yaml files
      for (const auto& entry :
           std::filesystem::directory_iterator(yaml_fp.parent_path())) {
        if (entry.is_regular_file() && (entry.path().extension() == ".yml")) {
          std::filesystem::remove(entry);
          CTL_INF("Removed: " << entry.path().string());
        }
      }
    }

    // Remove model files if they are not imported locally
    if (model_entry.value().branch_name != "imported" &&
        !engine_svc_->IsRemoteEngine(mc.engine)) {
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
    if (db_service_->DeleteModelEntry(model_handle)) {
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
    const std::string& model_handle, const Json::Value& params_override,
    bool bypass_model_check) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  config::YamlHandler yaml_handler;
  std::optional<std::string> custom_prompt_template;
  std::optional<int> ctx_len;
  if (auto& o = params_override["prompt_template"]; !o.isNull()) {
    custom_prompt_template = o.asString();
  }

  if (auto& o = params_override["ctx_len"]; !o.isNull()) {
    ctx_len = o.asInt();
  }

  try {
    constexpr const int kDefautlContextLength = 8192;
    int max_model_context_length = kDefautlContextLength;
    Json::Value json_data;
    // Currently we don't support download vision models, so we need to bypass check
    if (!bypass_model_check) {
      auto result = db_service_->GetModelInfo(model_handle);
      if (result.has_error()) {
        CTL_WRN("Error: " + result.error());
        return cpp::fail(result.error());
      }
      auto model_entry = result.value();

      if (model_entry.engine == kVllmEngine) {
        Json::Value json_data;
        json_data["model"] = model_handle;
        json_data["engine"] = kVllmEngine;
        auto [status, data] =
            inference_svc_->LoadModel(std::make_shared<Json::Value>(json_data));

        auto status_code = status["status_code"].asInt();
        if (status_code == drogon::k200OK) {
          return StartModelResult{true, ""};
        } else if (status_code == drogon::k409Conflict) {
          CTL_INF("Model '" + model_handle + "' is already loaded");
          return StartModelResult{true, ""};
        } else {
          return cpp::fail("Model failed to start: " +
                           data["message"].asString());
        }
      }

      yaml_handler.ModelConfigFromFile(
          fmu::ToAbsoluteCortexDataPath(
              fs::path(model_entry.path_to_model_yaml))
              .string());
      auto mc = yaml_handler.GetModelConfig();

      // Running remote model
      if (engine_svc_->IsRemoteEngine(mc.engine)) {
        (void)engine_svc_->LoadEngine(mc.engine);
        config::RemoteModelConfig remote_mc;
        remote_mc.LoadFromYamlFile(fmu::ToAbsoluteCortexDataPath(
                                       fs::path(model_entry.path_to_model_yaml))
                                       .string());
        auto result = engine_svc_->GetEngineByNameAndVariant(mc.engine);
        if (result.has_error()) {
          CTL_WRN("Remote engine error: " + result.error());
          return cpp::fail(result.error());
        }
        auto remote_engine_json = result.value().ToJson();
        json_data = remote_mc.ToJson();

        json_data["api_key"] = std::move(remote_engine_json["api_key"]);
        if (auto v = remote_engine_json["version"].asString();
            !v.empty() && v != "latest") {
          json_data["version"] = v;
        }
        json_data["model_path"] = fmu::ToAbsoluteCortexDataPath(
                                      fs::path(model_entry.path_to_model_yaml))
                                      .string();
        json_data["metadata"] = std::move(remote_engine_json["metadata"]);

        auto ir =
            inference_svc_->LoadModel(std::make_shared<Json::Value>(json_data));
        auto status = std::get<0>(ir)["status_code"].asInt();
        auto data = std::get<1>(ir);
        if (status == drogon::k200OK) {
          return StartModelResult{/* .success = */ true, /* .warning = */ ""};
        } else if (status == drogon::k409Conflict) {
          CTL_INF("Model '" + model_handle + "' is already loaded");
          return StartModelResult{/* .success = */ true, /* .warning = */ ""};
        } else {
          // only report to user the error
          CTL_ERR("Model failed to start with status code: " << status);
          return cpp::fail("Model failed to start: " +
                           data["message"].asString());
        }
      }

      // end hard code

      json_data = mc.ToJson();
      if (mc.files.size() > 0) {
#if defined(_WIN32)
        json_data["model_path"] = cortex::wc::WstringToUtf8(
            fmu::ToAbsoluteCortexDataPath(fs::path(mc.files[0])).wstring());
#else
        json_data["model_path"] =
            fmu::ToAbsoluteCortexDataPath(fs::path(mc.files[0])).string();
#endif
      } else {
        LOG_WARN << "model_path is empty";
        return StartModelResult{/* .success = */ false, ""};
      }
      if (!mc.mmproj.empty()) {
#if defined(_WIN32)
        json_data["mmproj"] = cortex::wc::WstringToUtf8(
            fmu::ToAbsoluteCortexDataPath(fs::path(mc.mmproj)).wstring());
#else
        json_data["mmproj"] =
            fmu::ToAbsoluteCortexDataPath(fs::path(mc.mmproj)).string();
#endif
      }
      json_data["system_prompt"] = mc.system_template;
      json_data["user_prompt"] = mc.user_template;
      json_data["ai_prompt"] = mc.ai_template;
      json_data["ctx_len"] = std::min(kDefautlContextLength, mc.ctx_len);
      json_data["max_tokens"] = std::min(kDefautlContextLength, mc.ctx_len);
      max_model_context_length = mc.ctx_len;
    } else {
      bypass_stop_check_set_.insert(model_handle);
    }

    json_data["model"] = model_handle;
    if (auto& cpt = custom_prompt_template; !cpt.value_or("").empty()) {
      auto parse_prompt_result = string_utils::ParsePrompt(cpt.value());
      json_data["system_prompt"] = parse_prompt_result.system_prompt;
      json_data["user_prompt"] = parse_prompt_result.user_prompt;
      json_data["ai_prompt"] = parse_prompt_result.ai_prompt;
    }

    json_helper::MergeJson(json_data, params_override);

    // Set default cpu_threads if it is not configured
    if (!json_data.isMember("cpu_threads")) {
      json_data["cpu_threads"] = GetCpuThreads();
    }

    // Set the latest ctx_len
    if (ctx_len) {
      json_data["ctx_len"] =
          std::min(ctx_len.value(), max_model_context_length);
      json_data["max_tokens"] =
          std::min(ctx_len.value(), max_model_context_length);
    }
    CTL_INF(json_data.toStyledString());
    auto may_fallback_res = MayFallbackToCpu(json_data["model_path"].asString(),
                                             json_data["ngl"].asInt(),
                                             json_data["ctx_len"].asInt());
    if (may_fallback_res.has_error()) {
      return cpp::fail(may_fallback_res.error());
    }

    assert(!!inference_svc_);

    auto ir =
        inference_svc_->LoadModel(std::make_shared<Json::Value>(json_data));
    auto status = std::get<0>(ir)["status_code"].asInt();
    auto data = std::get<1>(ir);

    if (status == drogon::k200OK) {
      // start model successfully, in case not vision model, we store the metadata so we can use
      // for each inference
      if (!json_data.isMember("mmproj") || json_data["mmproj"].isNull()) {
        auto metadata_res = GetModelMetadata(model_handle);
        if (metadata_res.has_value()) {
          loaded_model_metadata_map_.emplace(model_handle,
                                             std::move(metadata_res.value()));
          CTL_INF("Successfully stored metadata for model " << model_handle);
        } else {
          CTL_WRN("Failed to get metadata for model " << model_handle << ": "
                                                      << metadata_res.error());
        }
      }

      return StartModelResult{/* .success = */ true,
                              /* .warning = */ may_fallback_res.value()};
    } else if (status == drogon::k409Conflict) {
      CTL_INF("Model '" + model_handle + "' is already loaded");
      return StartModelResult{
          /* .success = */ true,
          /* .warning = */ may_fallback_res.value_or(std::nullopt)};
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
  config::YamlHandler yaml_handler;

  try {
    auto bypass_check = (bypass_stop_check_set_.find(model_handle) !=
                         bypass_stop_check_set_.end());
    std::string engine_name = "";
    if (!bypass_check) {
      auto result = db_service_->GetModelInfo(model_handle);
      if (result.has_error()) {
        CTL_WRN("Error: " + result.error());
        return cpp::fail(result.error());
      }

      const auto model_entry = result.value();
      if (model_entry.engine == kVllmEngine) {
        engine_name = kVllmEngine;
      } else {
        yaml_handler.ModelConfigFromFile(
            fmu::ToAbsoluteCortexDataPath(
                fs::path(model_entry.path_to_model_yaml))
                .string());
        auto mc = yaml_handler.GetModelConfig();
        engine_name = mc.engine;
      }
    }
    if (bypass_check) {
      engine_name = kLlamaEngine;
    }

    //
    assert(inference_svc_);
    auto ir = inference_svc_->UnloadModel(engine_name, model_handle);
    auto status = std::get<0>(ir)["status_code"].asInt();
    auto data = std::get<1>(ir);
    if (status == drogon::k200OK) {
      if (bypass_check) {
        bypass_stop_check_set_.erase(model_handle);
      }
      loaded_model_metadata_map_.erase(model_handle);
      CTL_INF("Removed metadata for model " << model_handle);
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
  config::YamlHandler yaml_handler;

  try {
    auto model_entry = db_service_->GetModelInfo(model_handle);
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
    if (status == drogon::k200OK) {
      return true;
    } else {
      return cpp::fail(data["message"].asString());
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
    if (url_obj.has_error() || url_obj->pathParams.size() < 5) {
      return cpp::fail(
          "Invalid url: " + input +
          ", a valid URL example is: "
          "https://huggingface.co/cortexso/tinyllama/blob/1b/model.gguf");
    }
    if (url_obj->host == kHuggingFaceHost) {
      if (url_obj->pathParams[2] == "blob") {
        url_obj->pathParams[2] = "resolve";
      }
    } else {
      return cpp::fail("Only support pull model from " +
                       std::string(kHuggingFaceHost));
    }

    auto author{url_obj->pathParams[0]};
    auto model_id{url_obj->pathParams[1]};
    auto file_name{url_obj->pathParams.back()};
    if (author == "cortexso") {
      return ModelPullInfo{
          /* .id = */ model_id + ":" + url_obj->pathParams[3],
          /* .default_branch = */ "main",
          /* .downloaded_models = */ {},
          /* .available_models = */ {},
          /* .model_source = */ "",
          /* .download_url = */ url_parser::FromUrl(url_obj.value())};
    }
    return ModelPullInfo{
        /* .id = */ author + ":" + model_id + ":" + file_name,
        /* .default_branch = */ "main",
        /* .downloaded_models = */ {},
        /* .available_models = */ {},
        /* .model_source = */ "",
        /* .download_url = */ url_parser::FromUrl(url_obj.value())};
  }

  if (input.find(":") != std::string::npos) {
    auto parsed = string_utils::SplitBy(input, ":");
    if (parsed.size() != 2 && parsed.size() != 3) {
      return cpp::fail("Invalid model handle: " + input);
    }
    return ModelPullInfo{/* .id = */ input,
                         /* .default_branch = */ "main",
                         /* .downloaded_models = */ {},
                         /* .available_models = */ {},
                         /* .model_source = */ "",
                         /* .download_url = */ input};
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
        return cpp::fail("Model not found on " + std::string{kHuggingFaceHost});
      }

      // repo containing GGUF files
      if (repo_info->gguf.has_value()) {
        std::vector<std::string> options{};
        for (const auto& sibling : repo_info->siblings) {
          if (string_utils::EndsWith(sibling.rfilename, ".gguf")) {
            options.push_back(sibling.rfilename);
          }
        }

        return ModelPullInfo{
          /* .id = */ author + ":" + model_name,
          /* .default_branch = */ "main",
          /* .downloaded_models = */ {},
          /* .available_models = */ options,
          /* .model_source = */ "",
          /* .download_url = */
          huggingface_utils::GetDownloadableUrl(author, model_name, "")};
      }

      // repo that is supported by HF transformers
      // we will download the whole repo
      if (repo_info->library_name.value_or("") == "transformers") {
        return ModelPullInfo{
          /* .id = */ author + "/" + model_name,
          /* .default_branch = */ "main",
          /* .downloaded_models = */ {},
          /* .available_models = */ {},
          /* .model_source = */ "huggingface",
          /* .download_url = */ ""};
      }

      return cpp::fail(
          "Unsupported model. Currently, only GGUF models and HF models are "
          "supported.");
    }
  }
  auto branches =
      huggingface_utils::GetModelRepositoryBranches("cortexso", model_name);
  if (branches.has_error()) {
    return cpp::fail(branches.error());
  }

  auto default_model_branch = huggingface_utils::GetDefaultBranch(model_name);

  auto downloaded_model_ids = db_service_->FindRelatedModel(model_name)
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

  return ModelPullInfo{
      /* .id = */ model_name,
      /* .default_branch = */ normalized_def_branch.value_or(""),
      /* .downloaded_models = */ downloaded_model_ids,
      /* .available_models = */ avai_download_opts,
      /* .model_source = */ "cortexso",
      /* .download_url = */ ""};
}

cpp::result<DownloadTask, std::string> ModelService::PullModel(
    const std::string& model_handle,
    const std::optional<std::string>& desired_model_id,
    const std::optional<std::string>& desired_model_name) {
  CTL_INF("Handle model input, model handle: " + model_handle);

  if (string_utils::StartsWith(model_handle, "https"))
    return HandleDownloadUrlAsync(model_handle, desired_model_id,
                                  desired_model_name);

  // HF model handle
  if (model_handle.find("/") != std::string::npos) {
    const auto author_model = string_utils::SplitBy(model_handle, "/");
    if (author_model.size() != 2)
      return cpp::fail("Invalid model handle");

    return DownloadHfModelAsync(author_model[0], author_model[1]);
  }

  if (model_handle.find(":") == std::string::npos)
    return cpp::fail("Invalid model handle or not supported!");

  auto model_and_branch = string_utils::SplitBy(model_handle, ":");

  // cortexso format - model:branch
  if (model_and_branch.size() == 2)
    return DownloadModelFromCortexsoAsync(
        model_and_branch[0], model_and_branch[1], desired_model_id);

  if (model_and_branch.size() == 3) {
    // single GGUF file
    // author_id:model_name:filename
    auto mh = url_parser::Url{
        .protocol = "https",
        .host = kHuggingFaceHost,
        .pathParams = {
            model_and_branch[0],
            model_and_branch[1],
            "resolve",
            "main",
            model_and_branch[2],
        }}.ToFullPath();
    return HandleDownloadUrlAsync(mh, desired_model_id, desired_model_name);
  }

  return cpp::fail("Invalid model handle or not supported!");
}

cpp::result<std::string, std::string> ModelService::AbortDownloadModel(
    const std::string& task_id) {
  return download_service_->StopTask(task_id);
}

cpp::result<std::optional<std::string>, std::string>
ModelService::MayFallbackToCpu(const std::string& model_path, int ngl,
                               int ctx_len, int n_batch, int n_ubatch,
                               const std::string& kv_cache_type) {
  // TODO(sang) temporary disable this function
  return std::nullopt;
  assert(hw_service_);
  auto hw_info = hw_service_->GetHardwareInfo();
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
          [](const EngineVariantResponse& e1, const EngineVariantResponse& e2) {
            return e1.name > e2.name;
          });
      for (auto& e : es) {
        CTL_INF(e.name << " " << e.version << " " << e.engine);
        // Select the first CPU candidate
        if (e.name.find("cuda") == std::string::npos) {
          auto r = engine_svc_->SetDefaultEngineVariant(kLlamaEngine, e.version,
                                                        e.name);
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
  int64_t free_vram_MiB = 0;
  for (const auto& gpu : hw_info.gpus) {
    free_vram_MiB += gpu.free_vram;
  }

  auto free_ram_MiB = hw_info.ram.available_MiB;

#if defined(__APPLE__) && defined(__MACH__)
  free_vram_MiB = free_ram_MiB;
#endif

  hardware::RunConfig rc = {/* .ngl = */ ngl,
                            /* .ctx_len = */ ctx_len,
                            /* .n_batch = */ n_batch,
                            /* .n_ubatch = */ n_ubatch,
                            /* .kv_cache_type = */ kv_cache_type,
                            /* .free_vram_MiB = */ free_vram_MiB};
  auto es = hardware::EstimateLLaMACppRun(model_path, rc);

  if (!!es && (*es).gpu_mode.vram_MiB > free_vram_MiB && is_cuda) {
    CTL_WRN("Not enough VRAM - " << "required: " << (*es).gpu_mode.vram_MiB
                                 << ", available: " << free_vram_MiB);
  }

  if (!!es && (*es).cpu_mode.ram_MiB > free_ram_MiB) {
    CTL_WRN("Not enough RAM - " << "required: " << (*es).cpu_mode.ram_MiB
                                << ", available: " << free_ram_MiB);
  }

  return warning;
}

int ModelService::GetCpuThreads() const {
  return std::max(std::thread::hardware_concurrency() / 2, 1u);
}

cpp::result<std::shared_ptr<ModelMetadata>, std::string>
ModelService::GetModelMetadata(const std::string& model_id) const {
  if (model_id.empty()) {
    return cpp::fail("Model ID can't be empty");
  }

  auto model_config = GetDownloadedModel(model_id);
  if (!model_config.has_value()) {
    return cpp::fail("Can't get model config for " + model_id);
  }

  if (model_config->files.empty()) {
    return cpp::fail("Model has no actual file. Might not be a local model!");
  }
  // TODO: handle the case we have multiple files
  auto file = model_config->files[0];

  auto model_metadata_res = cortex_utils::ReadGgufMetadata(
      file_manager_utils::ToAbsoluteCortexDataPath(
          std::filesystem::path(file)));
  if (!model_metadata_res.has_value()) {
    CTL_ERR("Failed to read metadata: " + model_metadata_res.error());
    return cpp::fail("Failed to read metadata: " + model_metadata_res.error());
  }
  return std::move(*model_metadata_res);
}

std::shared_ptr<ModelMetadata> ModelService::GetCachedModelMetadata(
    const std::string& model_id) const {
  if (loaded_model_metadata_map_.find(model_id) ==
      loaded_model_metadata_map_.end())
    return nullptr;
  return loaded_model_metadata_map_.at(model_id);
}

std::string ModelService::GetEngineByModelId(
    const std::string& model_id) const {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  auto model_entry = db_service_->GetModelInfo(model_id);
  if (model_entry.has_error()) {
    CTL_WRN("Error: " + model_entry.error());
    return "";
  }
  config::YamlHandler yaml_handler;
  yaml_handler.ModelConfigFromFile(
      fmu::ToAbsoluteCortexDataPath(
          fs::path(model_entry.value().path_to_model_yaml))
          .string());
  auto mc = yaml_handler.GetModelConfig();
  CTL_DBG(mc.engine);
  return mc.engine;
}

void ModelService::ProcessBgrTasks() {
  CTL_INF("Start processing background tasks")
  auto cb = [this] {
    CTL_DBG("Estimate model resource usage");
    auto list_entry = db_service_->LoadModelList();
    if (list_entry) {
      for (const auto& model_entry : list_entry.value()) {
        // Only process local models
        if (model_entry.status == cortex::db::ModelStatus::Downloaded) {
          auto es = EstimateModel(model_entry.model);
          if (es.has_value()) {
            std::lock_guard l(es_mtx_);
            es_[model_entry.model] = es.value();
          }
        }
      }
    }
  };

  auto clone = cb;
  task_queue_.RunInQueue(std::move(cb));
  task_queue_.RunEvery(std::chrono::seconds(60), std::move(clone));
}
