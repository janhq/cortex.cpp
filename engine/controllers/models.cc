#include "models.h"
#include "commands/model_del_cmd.h"
#include "config/yaml_config.h"
#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/model_callback_utils.h"
#include "utils/modellist_utils.h"

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

  // Iterate through directory

  try {
    modellist_utils::ModelListUtils modellist_handler;
    config::YamlHandler yaml_handler;

    auto list_entry = modellist_handler.LoadModelList();

    for (const auto& model_entry : list_entry) {
      // auto model_entry = modellist_handler.GetModelInfo(model_handle);
      try {

        yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
        auto model_config = yaml_handler.GetModelConfig();
        Json::Value obj;
        obj["id"] = model_config.id;
        obj["name"] = model_config.name;
        obj["model"] = model_config.model;
        obj["version"] = model_config.version;

        Json::Value stop_array(Json::arrayValue);
        for (const std::string& stop : model_config.stop)
          stop_array.append(stop);
        obj["stop"] = stop_array;

        obj["stream"] = model_config.stream;
        obj["top_p"] = model_config.top_p;
        obj["temperature"] = model_config.temperature;
        obj["frequency_penalty"] = model_config.frequency_penalty;
        obj["presence_penalty"] = model_config.presence_penalty;
        obj["max_tokens"] = static_cast<int>(model_config.max_tokens);

        // New fields
        obj["seed"] = model_config.seed;
        obj["dynatemp_range"] = model_config.dynatemp_range;
        obj["dynatemp_exponent"] = model_config.dynatemp_exponent;
        obj["top_k"] = model_config.top_k;
        obj["min_p"] = model_config.min_p;
        obj["tfs_z"] = model_config.tfs_z;
        obj["typ_p"] = model_config.typ_p;
        obj["repeat_last_n"] = model_config.repeat_last_n;
        obj["repeat_penalty"] = model_config.repeat_penalty;
        obj["mirostat"] = model_config.mirostat;
        obj["mirostat_tau"] = model_config.mirostat_tau;
        obj["mirostat_eta"] = model_config.mirostat_eta;
        obj["penalize_nl"] = model_config.penalize_nl;
        obj["ignore_eos"] = model_config.ignore_eos;
        obj["n_probs"] = model_config.n_probs;
        obj["min_keep"] = model_config.min_keep;

        obj["ngl"] = model_config.ngl;
        obj["ctx_len"] = static_cast<int>(model_config.ctx_len);
        obj["engine"] = model_config.engine;
        obj["prompt_template"] = model_config.prompt_template;
        obj["system_template"] = model_config.system_template;
        obj["user_template"] = model_config.user_template;
        obj["ai_template"] = model_config.ai_template;

        obj["os"] = model_config.os;
        obj["gpu_arch"] = model_config.gpu_arch;
        obj["quantization_method"] = model_config.quantization_method;
        obj["precision"] = model_config.precision;

        Json::Value files_array(Json::arrayValue);
        for (const std::string& file : model_config.files)
          files_array.append(file);
        obj["files"] = files_array;

        obj["created"] = static_cast<uint32_t>(model_config.created);
        obj["object"] = model_config.object;
        obj["owned_by"] = model_config.owned_by;
        obj["text_model"] = model_config.text_model;

        if (model_config.engine == "cortex.tensorrt-llm") {
          obj["trtllm_version"] = model_config.trtllm_version;
          obj["tp"] = model_config.tp;
        }

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
  } catch (const std::exception& e) {
    std::string message =
        "Fail to get list model information: " + std::string(e.what());
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
    modellist_utils::ModelListUtils modellist_handler;
    config::YamlHandler yaml_handler;
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
    yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
    auto model_config = yaml_handler.GetModelConfig();

    Json::Value obj;
    obj["id"] = model_config.id;
    obj["name"] = model_config.name;
    obj["model"] = model_config.model;
    obj["version"] = model_config.version;

    Json::Value stop_array(Json::arrayValue);
    for (const std::string& stop : model_config.stop)
      stop_array.append(stop);
    obj["stop"] = stop_array;

    obj["stream"] = model_config.stream;
    obj["top_p"] = model_config.top_p;
    obj["temperature"] = model_config.temperature;
    obj["frequency_penalty"] = model_config.frequency_penalty;
    obj["presence_penalty"] = model_config.presence_penalty;
    obj["max_tokens"] = static_cast<int>(model_config.max_tokens);

    // New fields
    obj["seed"] = model_config.seed;
    obj["dynatemp_range"] = model_config.dynatemp_range;
    obj["dynatemp_exponent"] = model_config.dynatemp_exponent;
    obj["top_k"] = model_config.top_k;
    obj["min_p"] = model_config.min_p;
    obj["tfs_z"] = model_config.tfs_z;
    obj["typ_p"] = model_config.typ_p;
    obj["repeat_last_n"] = model_config.repeat_last_n;
    obj["repeat_penalty"] = model_config.repeat_penalty;
    obj["mirostat"] = model_config.mirostat;
    obj["mirostat_tau"] = model_config.mirostat_tau;
    obj["mirostat_eta"] = model_config.mirostat_eta;
    obj["penalize_nl"] = model_config.penalize_nl;
    obj["ignore_eos"] = model_config.ignore_eos;
    obj["n_probs"] = model_config.n_probs;
    obj["min_keep"] = model_config.min_keep;

    obj["ngl"] = model_config.ngl;
    obj["ctx_len"] = static_cast<int>(model_config.ctx_len);
    obj["engine"] = model_config.engine;
    obj["prompt_template"] = model_config.prompt_template;
    obj["system_template"] = model_config.system_template;
    obj["user_template"] = model_config.user_template;
    obj["ai_template"] = model_config.ai_template;

    obj["os"] = model_config.os;
    obj["gpu_arch"] = model_config.gpu_arch;
    obj["quantization_method"] = model_config.quantization_method;
    obj["precision"] = model_config.precision;

    Json::Value files_array(Json::arrayValue);
    for (const std::string& file : model_config.files)
      files_array.append(file);
    obj["files"] = files_array;

    obj["created"] = static_cast<uint32_t>(model_config.created);
    obj["object"] = model_config.object;
    obj["owned_by"] = model_config.owned_by;
    obj["text_model"] = model_config.text_model;

    if (model_config.engine == "cortex.tensorrt-llm") {
      obj["trtllm_version"] = model_config.trtllm_version;
      obj["tp"] = model_config.tp;
    }

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
  modellist_utils::ModelListUtils modellist_utils_obj;

  std::string model_yaml_path = (file_manager_utils::GetModelsContainerPath() /
                                 std::filesystem::path("imported") /
                                 std::filesystem::path(modelHandle + ".yml"))
                                    .string();
  modellist_utils::ModelEntry model_entry{
      modelHandle,     "local",     "imported",
      model_yaml_path, modelHandle, modellist_utils::ModelStatus::READY};
  try {
    std::filesystem::create_directories(
        std::filesystem::path(model_yaml_path).parent_path());
    gguf_handler.Parse(modelPath);
    config::ModelConfig model_config = gguf_handler.GetModelConfig();
    model_config.files.push_back(modelPath);
    model_config.name = modelHandle;
    yaml_handler.UpdateModelConfig(model_config);

    if (modellist_utils_obj.AddModelEntry(model_entry)) {
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
    std::remove(model_yaml_path.c_str());
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

  modellist_utils::ModelListUtils modellist_handler;
  try {
    if (modellist_handler.UpdateModelAlias(model_handle, model_alias)) {
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