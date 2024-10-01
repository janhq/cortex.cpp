#include "models.h"
#include <drogon/HttpTypes.h>
#include "config/gguf_parser.h"
#include "config/yaml_config.h"
#include "database/models.h"
#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/http_util.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

void Models::PullModel(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!http_util::HasFieldInReq(req, callback, "modelId")) {
    return;
  }

  auto model_handle = (*(req->getJsonObject())).get("modelId", "").asString();
  if (model_handle.empty()) {
    Json::Value ret;
    ret["result"] = "Bad Request";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto handle_model_input =
      [&, model_handle]() -> cpp::result<std::string, std::string> {
    CTL_INF("Handle model input, model handle: " + model_handle);
    if (string_utils::StartsWith(model_handle, "https")) {
      return model_service_.HandleUrl(model_handle, true);
    } else if (model_handle.find(":") == std::string::npos) {
      auto model_and_branch = string_utils::SplitBy(model_handle, ":");
      return model_service_.DownloadModelFromCortexso(
          model_and_branch[0], model_and_branch[1], true);
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
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Models::ListModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  Json::Value ret;
  ret["object"] = "list";
  Json::Value data(Json::arrayValue);

  // Iterate through directory

  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;

  auto list_entry = modellist_handler.LoadModelList();
  if (list_entry) {
    for (const auto& model_entry : list_entry.value()) {
      // auto model_entry = modellist_handler.GetModelInfo(model_handle);
      try {

        yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
        auto model_config = yaml_handler.GetModelConfig();
        Json::Value obj = model_config.ToJson();

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

  try {
    cortex::db::Models modellist_handler;
    config::YamlHandler yaml_handler;
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    if (model_entry.has_error()) {
      // CLI_LOG("Error: " + model_entry.error());
      ret["data"] = data;
      ret["result"] = "Fail to get model information";
      ret["message"] = "Error: " + model_entry.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }
    yaml_handler.ModelConfigFromFile(model_entry.value().path_to_model_yaml);
    auto model_config = yaml_handler.GetModelConfig();

    Json::Value obj = model_config.ToJson();

    data.append(std::move(obj));
    ret["data"] = data;
    ret["result"] = "OK";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  } catch (const std::exception& e) {
    std::string message = "Fail to get model information with ID '" +
                          model_handle + "': " + e.what();
    LOG_ERROR << message;
    ret["data"] = data;
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
  auto result = model_service_.DeleteModel(model_id);
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

void Models::UpdateModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  if (!http_util::HasFieldInReq(req, callback, "modelId")) {
    return;
  }
  auto model_id = (*(req->getJsonObject())).get("modelId", "").asString();
  auto json_body = *(req->getJsonObject());
  try {
    cortex::db::Models model_list_utils;
    auto model_entry = model_list_utils.GetModelInfo(model_id);
    config::YamlHandler yaml_handler;
    yaml_handler.ModelConfigFromFile(model_entry.value().path_to_model_yaml);
    config::ModelConfig model_config = yaml_handler.GetModelConfig();
    model_config.FromJson(json_body);
    yaml_handler.UpdateModelConfig(model_config);
    yaml_handler.WriteYamlFile(model_entry.value().path_to_model_yaml);
    std::string message = "Successfully update model ID '" + model_id +
                          "': " + json_body.toStyledString();
    LOG_INFO << message;
    Json::Value ret;
    ret["result"] = "Updated successfully!";
    ret["modelHandle"] = model_id;
    ret["message"] = message;

    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
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
  if (!http_util::HasFieldInReq(req, callback, "modelId") ||
      !http_util::HasFieldInReq(req, callback, "modelPath")) {
    return;
  }
  auto modelHandle = (*(req->getJsonObject())).get("modelId", "").asString();
  auto modelPath = (*(req->getJsonObject())).get("modelPath", "").asString();
  config::GGUFHandler gguf_handler;
  config::YamlHandler yaml_handler;
  cortex::db::Models modellist_utils_obj;

  std::string model_yaml_path = (file_manager_utils::GetModelsContainerPath() /
                                 std::filesystem::path("imported") /
                                 std::filesystem::path(modelHandle + ".yml"))
                                    .string();
  cortex::db::ModelEntry model_entry{modelHandle, "local", "imported",
                                     model_yaml_path, modelHandle};
  try {
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

void Models::SetModelAlias(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  if (!http_util::HasFieldInReq(req, callback, "modelId") ||
      !http_util::HasFieldInReq(req, callback, "modelAlias")) {
    return;
  }
  auto model_handle = (*(req->getJsonObject())).get("modelId", "").asString();
  auto model_alias = (*(req->getJsonObject())).get("modelAlias", "").asString();
  LOG_DEBUG << "GetModel, Model handle: " << model_handle
            << ", Model alias: " << model_alias;

  cortex::db::Models modellist_handler;
  try {
    auto result = modellist_handler.UpdateModelAlias(model_handle, model_alias);
    if (result.has_error()) {
      std::string message = result.error();
      LOG_ERROR << message;
        Json::Value ret;
        ret["result"] = "Set alias failed!";
        ret["modelHandle"] = model_handle;
        ret["message"] = message;
        auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
    } else {
      if (result.value()) {
        std::string message = "Successfully set model alias '" + model_alias +
                              "' for modeID '" + model_handle + "'.";
        LOG_INFO << message;
        Json::Value ret;
        ret["result"] = "OK";
        ret["modelHandle"] = model_handle;
        ret["message"] = message;
        auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
        resp->setStatusCode(k200OK);
        callback(resp);
      } else {
        std::string message = "Unable to set model alias for modelID '" +
                              model_handle + "': model alias '" + model_alias +
                              "' is not unique!";
        LOG_ERROR << message;
        Json::Value ret;
        ret["result"] = "Set alias failed!";
        ret["modelHandle"] = model_handle;
        ret["message"] = message;
        auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
      }
    }
  } catch (const std::exception& e) {
    std::string message = "Error when setting model alias ('" + model_alias +
                          "') for modelID '" + model_handle + "':" + e.what();
    LOG_ERROR << message;
    Json::Value ret;
    ret["result"] = "Set alias failed!";
    ret["modelHandle"] = model_handle;
    ret["message"] = message;
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
  auto result = model_service_.StartModel(
      config.apiServerHost, std::stoi(config.apiServerPort), model_handle);
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
  auto result = model_service_.StopModel(
      config.apiServerHost, std::stoi(config.apiServerPort), model_handle);
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