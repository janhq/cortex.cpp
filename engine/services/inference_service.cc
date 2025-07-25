#include "inference_service.h"
#include <drogon/HttpTypes.h>
#include "utils/engine_constants.h"
#include "utils/function_calling/common.h"
#include "utils/jinja_utils.h"

cpp::result<void, InferResult> InferenceService::HandleChatCompletion(
    std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }
  CTL_DBG("engine_type: " << engine_type);
  auto tool_choice = json_body->get("tool_choice", Json::Value::null);
  auto model_id = json_body->get("model", "").asString();
  if (saved_models_.find(model_id) != saved_models_.end()) {
    // check if model is started, if not start it first
    Json::Value root;
    root["model"] = model_id;
    root["engine"] = engine_type;
    auto ir = GetModelStatus(std::make_shared<Json::Value>(root));
    auto status = std::get<0>(ir)["status_code"].asInt();
    if (status != drogon::k200OK) {
      CTL_INF("Model is not loaded, start loading it: " << model_id);
      // For remote engine, we use the updated configuration
      if (engine_service_->IsRemoteEngine(engine_type)) {
        (void)model_service_.lock()->StartModel(model_id, {}, false);
      } else {
        (void)LoadModel(saved_models_.at(model_id));
      }
    }
  }
  CTL_DBG("engine_type: " << engine_type);

  auto engine_result = engine_service_->GetLoadedEngine(engine_type);
  if (engine_result.has_error()) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    Json::Value stt;
    stt["status_code"] = drogon::k400BadRequest;
    LOG_WARN << "Engine is not loaded yet";
    return cpp::fail(std::make_pair(stt, res));
  }

  CTL_DBG("Json body inference: " + json_body->toStyledString());

  auto cb = [q, tool_choice](Json::Value status, Json::Value res) {
    if (!tool_choice.isNull()) {
      res["tool_choice"] = tool_choice;
    }
    q->push(std::make_pair(status, res));
  };
  if (std::holds_alternative<EngineI*>(engine_result.value())) {
    std::get<EngineI*>(engine_result.value())
        ->HandleChatCompletion(json_body, std::move(cb));
  } else {
    std::get<RemoteEngineI*>(engine_result.value())
        ->HandleChatCompletion(json_body, std::move(cb));
  }

  return {};
}

cpp::result<void, InferResult> InferenceService::HandleEmbedding(
    std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }

  auto engine_result = engine_service_->GetLoadedEngine(engine_type);
  if (engine_result.has_error()) {
    Json::Value res;
    Json::Value stt;
    res["message"] = "Engine is not loaded yet";
    stt["status_code"] = drogon::k400BadRequest;
    LOG_WARN << "Engine is not loaded yet";
    return cpp::fail(std::make_pair(stt, res));
  }

  auto cb = [q](Json::Value status, Json::Value res) {
    q->push(std::make_pair(status, res));
  };
  if (std::holds_alternative<EngineI*>(engine_result.value())) {
    std::get<EngineI*>(engine_result.value())
        ->HandleEmbedding(json_body, std::move(cb));
  } else {
    std::get<RemoteEngineI*>(engine_result.value())
        ->HandleEmbedding(json_body, std::move(cb));
  }
  return {};
}

InferResult InferenceService::LoadModel(
    std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }

  Json::Value r;
  Json::Value stt;
  auto load_engine_result = engine_service_->LoadEngine(engine_type);
  if (load_engine_result.has_error()) {
    LOG_ERROR << "Could not load engine: " << load_engine_result.error();

    r["message"] = "Could not load engine " + engine_type + ": " +
                   load_engine_result.error();
    stt["status_code"] = drogon::k500InternalServerError;
    return std::make_pair(stt, r);
  }

  // might need mutex here
  auto engine_result = engine_service_->GetLoadedEngine(engine_type);

  auto cb = [&stt, &r](Json::Value status, Json::Value res) {
    stt = status;
    r = res;
  };
  if (std::holds_alternative<EngineI*>(engine_result.value())) {
    std::get<EngineI*>(engine_result.value())
        ->LoadModel(json_body, std::move(cb));
  } else {
    std::get<RemoteEngineI*>(engine_result.value())
        ->LoadModel(json_body, std::move(cb));
  }
  // Save model config to reload if needed
  auto model_id = json_body->get("model", "").asString();
  saved_models_[model_id] = json_body;
  return std::make_pair(stt, r);
}

InferResult InferenceService::UnloadModel(const std::string& engine_name,
                                          const std::string& model_id) {
  Json::Value r;
  Json::Value stt;
  auto engine_result = engine_service_->GetLoadedEngine(engine_name);
  if (engine_result.has_error()) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    Json::Value stt;
    stt["status_code"] = drogon::k400BadRequest;
    LOG_WARN << "Engine is not loaded yet";
    return std::make_pair(stt, res);
  }

  Json::Value json_body;
  json_body["engine"] = engine_name;
  json_body["model"] = model_id;

  LOG_TRACE << "Start unload model";
  auto cb = [&r, &stt](Json::Value status, Json::Value res) {
    stt = status;
    r = res;
  };
  if (std::holds_alternative<EngineI*>(engine_result.value())) {
    std::get<EngineI*>(engine_result.value())
        ->UnloadModel(std::make_shared<Json::Value>(json_body), std::move(cb));
  } else {
    std::get<RemoteEngineI*>(engine_result.value())
        ->UnloadModel(std::make_shared<Json::Value>(json_body), std::move(cb));
  }

  return std::make_pair(stt, r);
}

InferResult InferenceService::GetModelStatus(
    std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }

  Json::Value r;
  Json::Value stt;
  auto engine_result = engine_service_->GetLoadedEngine(engine_type);
  if (engine_result.has_error()) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    Json::Value stt;
    stt["status_code"] = drogon::k400BadRequest;
    LOG_WARN << "Engine is not loaded yet";
    return std::make_pair(stt, res);
  }

  LOG_TRACE << "Start to get model status";

  auto cb = [&stt, &r](Json::Value status, Json::Value res) {
    stt = status;
    r = res;
  };
  if (std::holds_alternative<EngineI*>(engine_result.value())) {
    std::get<EngineI*>(engine_result.value())
        ->GetModelStatus(json_body, std::move(cb));
  } else {
    std::get<RemoteEngineI*>(engine_result.value())
        ->GetModelStatus(json_body, std::move(cb));
  }

  return std::make_pair(stt, r);
}

InferResult InferenceService::GetModels(
    std::shared_ptr<Json::Value> json_body) {
  Json::Value r;
  Json::Value stt;

  auto loaded_engines = engine_service_->GetLoadedEngines();
  if (loaded_engines.empty()) {
    r["message"] = "No engine is loaded yet";
    stt["status_code"] = drogon::k400BadRequest;
    return std::make_pair(stt, r);
  }

  LOG_TRACE << "Start to get models";
  Json::Value resp_data(Json::arrayValue);
  auto cb = [&resp_data](Json::Value status, Json::Value res) {
    for (auto r : res["data"]) {
      resp_data.append(r);
    }
    (void) status;
  };
  for (const auto& loaded_engine : loaded_engines) {
    if (std::holds_alternative<EngineI*>(loaded_engine)) {
      auto e = std::get<EngineI*>(loaded_engine);
      e->GetModels(json_body, std::move(cb));
    } else {
      std::get<RemoteEngineI*>(loaded_engine)
          ->GetModels(json_body, std::move(cb));
    }
  }

  Json::Value root;
  root["data"] = resp_data;
  root["object"] = "list";
  stt["status_code"] = drogon::k200OK;
  return std::make_pair(stt, root);
}

bool InferenceService::StopInferencing(const std::string& engine_name,
                                       const std::string& model_id) {
  CTL_DBG("Stop inferencing");
  auto engine_result = engine_service_->GetLoadedEngine(engine_name);
  if (engine_result.has_error()) {
    LOG_WARN << "Engine is not loaded yet";
    return false;
  }

  if (std::holds_alternative<EngineI*>(engine_result.value())) {
    auto engine = std::get<EngineI*>(engine_result.value());
    engine->StopInferencing(model_id);
    CTL_INF("Stopped inferencing");
  }
  return true;
}

bool InferenceService::HasFieldInReq(std::shared_ptr<Json::Value> json_body,
                                     const std::string& field) {
  if (!json_body || (*json_body)[field].isNull()) {
    return false;
  }
  return true;
}

std::string InferenceService::GetEngineByModelId(
    const std::string& model_id) const {
  return model_service_.lock()->GetEngineByModelId(model_id);
}
