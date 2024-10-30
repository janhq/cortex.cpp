#include "engines.h"
#include <utility>
#include "services/engine_service.h"
#include "utils/archive_utils.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

void Engines::InstallEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) {

  if (engine.empty()) {
    Json::Value res;
    res["message"] = "Engine name is required";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "No engine field in path param";
    return;
  }

  auto version = (*(req->getJsonObject())).get("version", "latest").asString();
  auto result = engine_service_->InstallEngineAsync(engine, version);
  if (result.has_error()) {
    Json::Value res;
    res["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    Json::Value res;
    res["message"] = "Engine " + engine + " starts installing!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Engines::ListEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  auto status_list = engine_service_->GetEngineInfoList();

  Json::Value ret;
  ret["object"] = "list";
  Json::Value data(Json::arrayValue);
  for (auto& status : status_list) {
    Json::Value ret;
    ret["name"] = status.name;
    ret["description"] = status.description;
    ret["version"] = status.version.value_or("");
    ret["variant"] = status.variant.value_or("");
    ret["productName"] = status.product_name;
    ret["status"] = status.status;
    ret["format"] = status.format;

    data.append(std::move(ret));
  }

  ret["data"] = data;
  ret["result"] = "OK";
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Engines::GetEngine(const HttpRequestPtr& req,
                        std::function<void(const HttpResponsePtr&)>&& callback,
                        const std::string& engine) const {
  auto status = engine_service_->GetEngineInfo(engine);
  Json::Value ret;
  if (status.has_value()) {
    ret["name"] = status->name;
    ret["description"] = status->description;
    ret["version"] = status->version.value_or("");
    ret["variant"] = status->variant.value_or("");
    ret["productName"] = status->product_name;
    ret["status"] = status->status;
    ret["format"] = status->format;

    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  } else {
    Json::Value ret;
    ret["message"] = "Engine not found";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  }
}

void Engines::UninstallEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) {

  auto result = engine_service_->UninstallEngine(engine);
  Json::Value ret;

  if (result.has_error()) {
    CTL_INF(result.error());
    ret["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    CTL_INF("Engine uninstalled successfully");
    ret["message"] = "Engine " + engine + " uninstalled successfully!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}
