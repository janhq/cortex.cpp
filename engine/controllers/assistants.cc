#include "assistants.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

void Assistants::RetrieveAssistant(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& assistant_id) const {
  CTL_INF("RetrieveAssistant: " + assistant_id);
  auto res = assistant_service_->RetrieveAssistant(assistant_id);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    auto to_json_res = res->ToJson();
    if (to_json_res.has_error()) {
      CTL_ERR("Failed to convert assistant to json: " + to_json_res.error());
      Json::Value ret;
      ret["message"] = to_json_res.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
    } else {
      // TODO: namh need to use the text response because it contains model config
      auto resp =
          cortex_utils::CreateCortexHttpJsonResponse(res->ToJson().value());
      resp->setStatusCode(k200OK);
      callback(resp);
    }
  }
}

void Assistants::CreateAssistant(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& assistant_id) {
  auto json_body = req->getJsonObject();
  if (json_body == nullptr) {
    Json::Value ret;
    ret["message"] = "Request body can't be empty";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // Parse assistant from request body
  auto assistant_result = OpenAi::JanAssistant::FromJson(std::move(*json_body));
  if (assistant_result.has_error()) {
    Json::Value ret;
    ret["message"] = "Failed to parse assistant: " + assistant_result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // Call assistant service to create
  auto create_result = assistant_service_->CreateAssistant(
      assistant_id, assistant_result.value());
  if (create_result.has_error()) {
    Json::Value ret;
    ret["message"] = create_result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // Convert result to JSON and send response
  auto to_json_result = create_result->ToJson();
  if (to_json_result.has_error()) {
    CTL_ERR("Failed to convert assistant to json: " + to_json_result.error());
    Json::Value ret;
    ret["message"] = to_json_result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto resp =
      cortex_utils::CreateCortexHttpJsonResponse(to_json_result.value());
  resp->setStatusCode(k201Created);
  callback(resp);
}

void Assistants::ModifyAssistant(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& assistant_id) {
  auto json_body = req->getJsonObject();
  if (json_body == nullptr) {
    Json::Value ret;
    ret["message"] = "Request body can't be empty";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // Parse assistant from request body
  auto assistant_result = OpenAi::JanAssistant::FromJson(std::move(*json_body));
  if (assistant_result.has_error()) {
    Json::Value ret;
    ret["message"] = "Failed to parse assistant: " + assistant_result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // Call assistant service to create
  auto modify_result = assistant_service_->ModifyAssistant(
      assistant_id, assistant_result.value());
  if (modify_result.has_error()) {
    Json::Value ret;
    ret["message"] = modify_result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // Convert result to JSON and send response
  auto to_json_result = modify_result->ToJson();
  if (to_json_result.has_error()) {
    CTL_ERR("Failed to convert assistant to json: " + to_json_result.error());
    Json::Value ret;
    ret["message"] = to_json_result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto resp =
      cortex_utils::CreateCortexHttpJsonResponse(to_json_result.value());
  resp->setStatusCode(k200OK);
  callback(resp);
}
