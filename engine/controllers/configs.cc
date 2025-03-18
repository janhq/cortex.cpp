#include "configs.h"
#include "utils/cortex_utils.h"

void Configs::GetConfigurations(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  auto get_config_result = config_service_->GetApiServerConfiguration();
  if (get_config_result.has_error()) {
    Json::Value error_json;
    error_json["message"] = get_config_result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(error_json);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }

  auto resp = cortex_utils::CreateCortexHttpJsonResponse(
      get_config_result.value().ToJson());
  resp->setStatusCode(drogon::k200OK);
  callback(resp);
  (void)req;
  return;
}

void Configs::UpdateConfigurations(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto json_body = req->getJsonObject();
  if (json_body == nullptr) {
    Json::Value error_json;
    error_json["message"] = "Configuration must be provided via JSON body";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(error_json);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }
  auto update_config_result =
      config_service_->UpdateApiServerConfiguration(*json_body);
  if (update_config_result.has_error()) {
    Json::Value error_json;
    error_json["message"] = update_config_result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(error_json);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }

  Json::Value root;
  root["message"] = "Configuration updated successfully";
  root["config"] = update_config_result.value().ToJson();
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(root);
  resp->setStatusCode(drogon::k200OK);
  callback(resp);
  return;
}
