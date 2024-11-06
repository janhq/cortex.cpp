#include "configs.h"

void Configs::GetConfigurations(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  auto get_config_result = config_service_->GetApiServerConfiguration();
  if (get_config_result.has_error()) {
    Json::Value error_json;
    error_json["error"] = get_config_result.error();
    auto resp = drogon::HttpResponse::newHttpJsonResponse(error_json);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }

  auto resp = drogon::HttpResponse::newHttpJsonResponse(
      get_config_result.value().ToJson());
  resp->setStatusCode(drogon::k200OK);
  callback(resp);
  return;
}

void Configs::UpdateConfigurations(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto json_body = req->getJsonObject();
  if (!json_body) {
    Json::Value error_json;
    error_json["error"] = "Configuration must be provided via JSON body";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(error_json);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }
  auto update_config_result =
      config_service_->UpdateApiServerConfiguration(*json_body);
  if (update_config_result.has_error()) {
    Json::Value error_json;
    error_json["error"] = update_config_result.error();
    auto resp = drogon::HttpResponse::newHttpJsonResponse(error_json);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }

  Json::Value root;
  root["message"] = "Configuration updated successfully";
  root["config"] = update_config_result.value().ToJson();
  auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
  resp->setStatusCode(drogon::k200OK);
  callback(resp);
  return;
}
