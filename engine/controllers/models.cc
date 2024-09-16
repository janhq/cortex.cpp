#include "models.h"
#include "commands/model_del_cmd.h"
#include "config/yaml_config.h"
#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/model_callback_utils.h"

void Models::PullModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  if (!http_util::HasFieldInReq(req, callback, "modelId")) {
    return;
  }
  auto modelHandle = (*(req->getJsonObject())).get("modelId", "").asString();
  LOG_DEBUG << "PullModel, Model handle: " << modelHandle;
  if (modelHandle.empty()) {
    Json::Value ret;
    ret["result"] = "Bad Request";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto downloadTask = cortexso_parser::getDownloadTask(modelHandle);
  if (downloadTask.has_value()) {
    DownloadService downloadService;
    downloadService.AddAsyncDownloadTask(downloadTask.value(),
                                         model_callback_utils::DownloadModelCb);

    Json::Value ret;
    ret["result"] = "OK";
    ret["modelHandle"] = modelHandle;
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  } else {
    Json::Value ret;
    ret["result"] = "Not Found";
    ret["modelHandle"] = modelHandle;
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k404NotFound);
    callback(resp);
  }
}

void Models::ListModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  Json::Value ret;
  ret["object"] = "list";
  Json::Value data(Json::arrayValue);
  auto models_path = file_manager_utils::GetModelsContainerPath();
  if (std::filesystem::exists(models_path) &&
      std::filesystem::is_directory(models_path)) {
    // Iterate through directory
    for (const auto& entry : std::filesystem::directory_iterator(models_path)) {
      if (entry.is_regular_file() && entry.path().extension() == ".yaml") {
        try {
          config::YamlHandler handler;
          handler.ModelConfigFromFile(entry.path().string());
          auto const& model_config = handler.GetModelConfig();
          Json::Value obj;
          obj["name"] = model_config.name;
          obj["model"] = model_config.model;
          obj["version"] = model_config.version;
          Json::Value stop_array(Json::arrayValue);
          for (const std::string& stop : model_config.stop)
            stop_array.append(stop);
          obj["stop"] = stop_array;
          obj["top_p"] = model_config.top_p;
          obj["temperature"] = model_config.temperature;
          obj["presence_penalty"] = model_config.presence_penalty;
          obj["max_tokens"] = model_config.max_tokens;
          obj["stream"] = model_config.stream;
          obj["ngl"] = model_config.ngl;
          obj["ctx_len"] = model_config.ctx_len;
          obj["engine"] = model_config.engine;
          obj["prompt_template"] = model_config.prompt_template;

          Json::Value files_array(Json::arrayValue);
          for (const std::string& file : model_config.files)
            files_array.append(file);
          obj["files"] = files_array;
          obj["id"] = model_config.id;
          obj["created"] = static_cast<uint32_t>(model_config.created);
          obj["object"] = model_config.object;
          obj["owned_by"] = model_config.owned_by;
          if (model_config.engine == "cortex.tensorrt-llm") {
            obj["trtllm_version"] = model_config.trtllm_version;
          }
          data.append(std::move(obj));
        } catch (const std::exception& e) {
          LOG_ERROR << "Error reading yaml file '" << entry.path().string()
                    << "': " << e.what();
        }
      }
    }
  }
  ret["data"] = data;
  ret["result"] = "OK";
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Models::GetModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  if (!http_util::HasFieldInReq(req, callback, "modelId")) {
    return;
  }
  auto model_handle = (*(req->getJsonObject())).get("modelId", "").asString();
  LOG_DEBUG << "GetModel, Model handle: " << model_handle;
  Json::Value ret;
  ret["object"] = "list";
  Json::Value data(Json::arrayValue);
  if (std::filesystem::exists(cortex_utils::models_folder) &&
      std::filesystem::is_directory(cortex_utils::models_folder)) {
    // Iterate through directory
    for (const auto& entry :
         std::filesystem::directory_iterator(cortex_utils::models_folder)) {
      if (entry.is_regular_file() && entry.path().extension() == ".yaml" &&
          entry.path().stem() == model_handle) {
        try {
          config::YamlHandler handler;
          handler.ModelConfigFromFile(entry.path().string());
          auto const& model_config = handler.GetModelConfig();
          Json::Value obj;
          obj["name"] = model_config.name;
          obj["model"] = model_config.model;
          obj["version"] = model_config.version;
          Json::Value stop_array(Json::arrayValue);
          for (const std::string& stop : model_config.stop)
            stop_array.append(stop);
          obj["stop"] = stop_array;
          obj["top_p"] = model_config.top_p;
          obj["temperature"] = model_config.temperature;
          obj["presence_penalty"] = model_config.presence_penalty;
          obj["max_tokens"] = model_config.max_tokens;
          obj["stream"] = model_config.stream;
          obj["ngl"] = model_config.ngl;
          obj["ctx_len"] = model_config.ctx_len;
          obj["engine"] = model_config.engine;
          obj["prompt_template"] = model_config.prompt_template;

          Json::Value files_array(Json::arrayValue);
          for (const std::string& file : model_config.files)
            files_array.append(file);
          obj["files"] = files_array;
          obj["id"] = model_config.id;
          obj["created"] = static_cast<uint32_t>(model_config.created);
          obj["object"] = model_config.object;
          obj["owned_by"] = model_config.owned_by;
          if (model_config.engine == "cortex.tensorrt-llm") {
            obj["trtllm_version"] = model_config.trtllm_version;
          }
          data.append(std::move(obj));
        } catch (const std::exception& e) {
          LOG_ERROR << "Error reading yaml file '" << entry.path().string()
                    << "': " << e.what();
        }
      }
    }
  }
  ret["data"] = data;
  ret["result"] = "OK";
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Models::DeleteModel(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& model_id) const {
  LOG_DEBUG << "DeleteModel, Model handle: " << model_id;
  commands::ModelDelCmd mdc;
  if (mdc.Exec(model_id)) {
    Json::Value ret;
    ret["result"] = "OK";
    ret["modelHandle"] = model_id;
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  } else {
    Json::Value ret;
    ret["result"] = "Not Found";
    ret["modelHandle"] = model_id;
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k404NotFound);
    callback(resp);
  }
}