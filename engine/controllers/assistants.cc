#include "assistants.h"
#include "common/api-dto/delete_success_response.h"
#include "common/dto/assistant_create_dto.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

void Assistants::RetrieveAssistant(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& assistant_id) const {
  const auto& headers = req->headers();
  auto it = headers.find(kOpenAiAssistantKeyV2);
  if (it != headers.end() && it->second == kOpenAiAssistantValueV2) {
    return RetrieveAssistantV2(req, std::move(callback), assistant_id);
  }

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

void Assistants::RetrieveAssistantV2(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& assistant_id) const {
  auto res = assistant_service_->RetrieveAssistantV2(assistant_id);

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
  (void) req;
}

void Assistants::CreateAssistantV2(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto json_body = req->getJsonObject();
  if (json_body == nullptr) {
    Json::Value ret;
    ret["message"] = "Request body can't be empty";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto dto = dto::CreateAssistantDto::FromJson(std::move(*json_body));
  CTL_INF("CreateAssistantV2: " << dto.model);
  auto validate_res = dto.Validate();
  if (validate_res.has_error()) {
    Json::Value ret;
    ret["message"] = validate_res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto res = assistant_service_->CreateAssistantV2(dto);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto to_json_res = res->ToJson();
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(to_json_res.value());
  resp->setStatusCode(k200OK);
  callback(resp);
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

void Assistants::ModifyAssistantV2(
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

  auto dto = dto::UpdateAssistantDto::FromJson(std::move(*json_body));
  auto validate_res = dto.Validate();
  if (validate_res.has_error()) {
    Json::Value ret;
    ret["message"] = validate_res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto res = assistant_service_->ModifyAssistantV2(assistant_id, dto);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto resp = cortex_utils::CreateCortexHttpJsonResponse(res->ToJson().value());
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Assistants::ModifyAssistant(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& assistant_id) {
  const auto& headers = req->headers();
  auto it = headers.find(kOpenAiAssistantKeyV2);
  if (it != headers.end() && it->second == kOpenAiAssistantValueV2) {
    return ModifyAssistantV2(req, std::move(callback), assistant_id);
  }
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

void Assistants::ListAssistants(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::optional<std::string> limit, std::optional<std::string> order,
    std::optional<std::string> after, std::optional<std::string> before) const {

  auto res = assistant_service_->ListAssistants(
      std::stoi(limit.value_or("20")), order.value_or("desc"),
      after.value_or(""), before.value_or(""));
  if (res.has_error()) {
    Json::Value root;
    root["message"] = res.error();
    auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
    response->setStatusCode(k400BadRequest);
    callback(response);
    return;
  }

  Json::Value assistant_list(Json::arrayValue);
  for (auto& msg : res.value()) {
    if (auto it = msg.ToJson(); it.has_value()) {
      assistant_list.append(it.value());
    } else {
      CTL_WRN("Failed to convert message to json: " + it.error());
    }
  }

  Json::Value root;
  root["object"] = "list";
  root["data"] = assistant_list;
  auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
  response->setStatusCode(k200OK);
  callback(response);
  (void) req;
}

void Assistants::DeleteAssistant(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& assistant_id) {
  auto res = assistant_service_->DeleteAssistantV2(assistant_id);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  api_response::DeleteSuccessResponse response;
  response.id = assistant_id;
  response.object = "assistant.deleted";
  response.deleted = true;
  auto resp =
      cortex_utils::CreateCortexHttpJsonResponse(response.ToJson().value());
  resp->setStatusCode(k200OK);
  callback(resp);
  (void) req;
}
