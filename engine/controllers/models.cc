#include "database/models.h"
#include <drogon/HttpTypes.h>
#include <optional>
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "models.h"
#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/http_util.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

void Models::PullModel(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!http_util::HasFieldInReq(req, callback, "model")) {
    return;
  }

  auto model_handle = (*(req->getJsonObject())).get("model", "").asString();
  if (model_handle.empty()) {
    Json::Value ret;
    ret["result"] = "Bad Request";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  std::optional<std::string> desired_model_id = std::nullopt;
  auto id = (*(req->getJsonObject())).get("id", "").asString();
  if (!id.empty()) {
    desired_model_id = id;
  }

  auto handle_model_input =
      [&, model_handle]() -> cpp::result<DownloadTask, std::string> {
    CTL_INF("Handle model input, model handle: " + model_handle);
    if (string_utils::StartsWith(model_handle, "https")) {
      return model_service_->HandleDownloadUrlAsync(model_handle,
                                                    desired_model_id);
    } else if (model_handle.find(":") != std::string::npos) {
      auto model_and_branch = string_utils::SplitBy(model_handle, ":");
      return model_service_->DownloadModelFromCortexsoAsync(
          model_and_branch[0], model_and_branch[1], desired_model_id);
    }

    return cpp::fail("Invalid model handle or not supported!");
  };

  auto result = handle_model_input();
  if (result.has_error()) {
    Json::Value ret;
    ret["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    Json::Value ret;
    ret["message"] = "Model start downloading!";
    ret["task"] = result.value().ToJsonCpp();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Models::AbortPullModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!http_util::HasFieldInReq(req, callback, "taskId")) {
    return;
  }
  auto task_id = (*(req->getJsonObject())).get("taskId", "").asString();
  if (task_id.empty()) {
    Json::Value ret;
    ret["result"] = "Bad Request";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto result = model_service_->AbortDownloadModel(task_id);
  if (result.has_error()) {
    Json::Value ret;
    ret["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    Json::Value ret;
    ret["message"] = "Download stopped successfully";
    ret["taskId"] = result.value();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Models::ListModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  Json::Value ret;
  ret["object"] = "list";
  Json::Value data(Json::arrayValue);

  // Iterate through directory

  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;

  auto list_entry = modellist_handler.LoadModelList();
  if (list_entry) {
    for (const auto& model_entry : list_entry.value()) {
      try {
        yaml_handler.ModelConfigFromFile(
            fmu::ToAbsoluteCortexDataPath(
                fs::path(model_entry.path_to_model_yaml))
                .string());
        auto model_config = yaml_handler.GetModelConfig();
        Json::Value obj = model_config.ToJson();
        obj["id"] = model_entry.model;
        obj["model"] = model_entry.model;
        data.append(std::move(obj));
        yaml_handler.Reset();
      } catch (const std::exception& e) {
        LOG_ERROR << "Failed to load yaml file for model: "
                  << model_entry.path_to_model_yaml << ", error: " << e.what();
      }
    }
    ret["data"] = data;
    ret["result"] = "OK";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  } else {
    std::string message = "Fail to get list model information: " +
                          std::string(list_entry.error());
    LOG_ERROR << message;
    ret["data"] = data;
    ret["result"] = "Fail to get list model information";
    ret["message"] = message;
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  }
}

void Models::GetModel(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback,
                      const std::string& model_id) const {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  LOG_DEBUG << "GetModel, Model handle: " << model_id;
  Json::Value ret;

  try {
    cortex::db::Models modellist_handler;
    config::YamlHandler yaml_handler;
    auto model_entry = modellist_handler.GetModelInfo(model_id);
    if (model_entry.has_error()) {
      ret["id"] = model_id;
      ret["object"] = "model";
      ret["result"] = "Fail to get model information";
      ret["message"] = "Error: " + model_entry.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }
    yaml_handler.ModelConfigFromFile(
        fmu::ToAbsoluteCortexDataPath(
            fs::path(model_entry.value().path_to_model_yaml))
            .string());
    auto model_config = yaml_handler.GetModelConfig();

    ret = model_config.ToJson();

    ret["id"] = model_config.model;
    ret["object"] = "model";
    ret["result"] = "OK";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  } catch (const std::exception& e) {
    std::string message =
        "Fail to get model information with ID '" + model_id + "': " + e.what();
    LOG_ERROR << message;
    ret["id"] = model_id;
    ret["object"] = "model";
    ret["result"] = "Fail to get model information";
    ret["message"] = message;
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  }
}

void Models::DeleteModel(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& model_id) {
  auto result = model_service_->DeleteModel(model_id);
  if (result.has_error()) {
    Json::Value ret;
    ret["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
  } else {
    Json::Value ret;
    ret["message"] = "Deleted successfully!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Models::UpdateModel(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& model_id) const {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  auto json_body = *(req->getJsonObject());
  try {
    cortex::db::Models model_list_utils;
    auto model_entry = model_list_utils.GetModelInfo(model_id);
    config::YamlHandler yaml_handler;
    auto yaml_fp = fmu::ToAbsoluteCortexDataPath(
        fs::path(model_entry.value().path_to_model_yaml));
    yaml_handler.ModelConfigFromFile(yaml_fp.string());
    config::ModelConfig model_config = yaml_handler.GetModelConfig();
    model_config.FromJson(json_body);
    yaml_handler.UpdateModelConfig(model_config);
    yaml_handler.WriteYamlFile(yaml_fp.string());
    std::string message = "Successfully update model ID '" + model_id +
                          "': " + json_body.toStyledString();
    LOG_INFO << message;
    Json::Value ret;
    ret["result"] = "Updated successfully!";
    ret["modelHandle"] = model_id;
    ret["message"] = message;

    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);

  } catch (const std::exception& e) {
    std::string error_message =
        "Error updating with model_id '" + model_id + "': " + e.what();
    LOG_ERROR << error_message;
    Json::Value ret;
    ret["result"] = "Updated failed!";
    ret["modelHandle"] = model_id;
    ret["message"] = error_message;

    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  }
}

void Models::ImportModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  if (!http_util::HasFieldInReq(req, callback, "model") ||
      !http_util::HasFieldInReq(req, callback, "modelPath")) {
    return;
  }
  auto modelHandle = (*(req->getJsonObject())).get("model", "").asString();
  auto modelPath = (*(req->getJsonObject())).get("modelPath", "").asString();
  config::GGUFHandler gguf_handler;
  config::YamlHandler yaml_handler;
  cortex::db::Models modellist_utils_obj;
  std::string model_yaml_path = (file_manager_utils::GetModelsContainerPath() /
                                 std::filesystem::path("imported") /
                                 std::filesystem::path(modelHandle + ".yml"))
                                    .string();

  try {
    // Use relative path for model_yaml_path. In case of import, we use absolute path for model
    auto yaml_rel_path =
        fmu::ToRelativeCortexDataPath(fs::path(model_yaml_path));
    cortex::db::ModelEntry model_entry{modelHandle, "local", "imported",
                                       yaml_rel_path.string(), modelHandle};

    std::filesystem::create_directories(
        std::filesystem::path(model_yaml_path).parent_path());
    gguf_handler.Parse(modelPath);
    config::ModelConfig model_config = gguf_handler.GetModelConfig();
    model_config.files.push_back(modelPath);
    model_config.model = modelHandle;
    yaml_handler.UpdateModelConfig(model_config);

    if (modellist_utils_obj.AddModelEntry(model_entry).value()) {
      yaml_handler.WriteYamlFile(model_yaml_path);
      std::string success_message = "Model is imported successfully!";
      LOG_INFO << success_message;
      Json::Value ret;
      ret["result"] = "OK";
      ret["modelHandle"] = modelHandle;
      ret["message"] = success_message;
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k200OK);
      callback(resp);

    } else {
      std::string error_message = "Fail to import model, model_id '" +
                                  modelHandle + "' already exists!";
      LOG_ERROR << error_message;
      Json::Value ret;
      ret["result"] = "Import failed!";
      ret["modelHandle"] = modelHandle;
      ret["message"] = error_message;

      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
    }

  } catch (const std::exception& e) {
    std::string error_message = "Error importing model path '" + modelPath +
                                "' with model_id '" + modelHandle +
                                "': " + e.what();
    LOG_ERROR << error_message;
    Json::Value ret;
    ret["result"] = "Import failed!";
    ret["modelHandle"] = modelHandle;
    ret["message"] = error_message;

    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  }
}

void Models::StartModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!http_util::HasFieldInReq(req, callback, "model"))
    return;
  auto config = file_manager_utils::GetCortexConfig();
  auto model_handle = (*(req->getJsonObject())).get("model", "").asString();
  StartParameterOverride params_override;
  if (auto& o = (*(req->getJsonObject()))["prompt_template"]; !o.isNull()) {
    params_override.custom_prompt_template = o.asString();
  }

  if (auto& o = (*(req->getJsonObject()))["cache_enabled"]; !o.isNull()) {
    params_override.cache_enabled = o.asBool();
  }

  if (auto& o = (*(req->getJsonObject()))["ngl"]; !o.isNull()) {
    params_override.ngl = o.asInt();
  }

  if (auto& o = (*(req->getJsonObject()))["n_parallel"]; !o.isNull()) {
    params_override.n_parallel = o.asInt();
  }

  if (auto& o = (*(req->getJsonObject()))["ctx_len"]; !o.isNull()) {
    params_override.ctx_len = o.asInt();
  }

  if (auto& o = (*(req->getJsonObject()))["cache_type"]; !o.isNull()) {
    params_override.cache_type = o.asString();
  }

  if (auto& o = (*(req->getJsonObject()))["mmproj"]; !o.isNull()) {
    params_override.mmproj = o.asString();
  }

  // Support both llama_model_path and model_path for backward compatible
  // model_path has higher priority
  if (auto& o = (*(req->getJsonObject()))["llama_model_path"]; !o.isNull()) {
    params_override.model_path = o.asString();
  }

  if (auto& o = (*(req->getJsonObject()))["model_path"]; !o.isNull()) {
    params_override.model_path = o.asString();
  }

  auto model_entry = model_service_->GetDownloadedModel(model_handle);
  if (!model_entry.has_value() && !params_override.bypass_model_check()) {
    Json::Value ret;
    ret["message"] = "Cannot find model: " + model_handle;
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }
  std::string engine_name = params_override.bypass_model_check()
                                ? kLlamaEngine
                                : model_entry.value().engine;
  auto engine_entry = engine_service_->GetEngineInfo(engine_name);
  if (engine_entry.has_error()) {
    Json::Value ret;
    ret["message"] = "Cannot find engine: " + engine_name;
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }
  if (engine_entry->status != "Ready") {
    Json::Value ret;
    ret["message"] = "Engine is not ready! Please install first!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }

  auto result = model_service_->StartModel(config.apiServerHost,
                                           std::stoi(config.apiServerPort),
                                           model_handle, params_override);
  if (result.has_error()) {
    Json::Value ret;
    ret["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
  } else {
    Json::Value ret;
    ret["message"] = "Started successfully!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Models::StopModel(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!http_util::HasFieldInReq(req, callback, "model"))
    return;
  auto config = file_manager_utils::GetCortexConfig();
  auto model_handle = (*(req->getJsonObject())).get("model", "").asString();
  auto result = model_service_->StopModel(
      config.apiServerHost, std::stoi(config.apiServerPort), model_handle);
  if (result.has_error()) {
    Json::Value ret;
    ret["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
  } else {
    Json::Value ret;
    ret["message"] = "Stopped successfully!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Models::GetModelStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& model_id) {
  auto config = file_manager_utils::GetCortexConfig();

  auto result = model_service_->GetModelStatus(
      config.apiServerHost, std::stoi(config.apiServerPort), model_id);
  if (result.has_error()) {
    Json::Value ret;
    ret["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
  } else {
    Json::Value ret;
    ret["message"] = "Model is running";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}
