#include "threads.h"
#include "common/api-dto/delete_success_response.h"
#include "common/variant_map.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

void Threads::ListThreads(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::optional<std::string> limit, std::optional<std::string> order,
    std::optional<std::string> after, std::optional<std::string> before) const {
	(void) req;
  CTL_INF("ListThreads");
  auto res = thread_service_->ListThreads(
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
  Json::Value msg_arr(Json::arrayValue);
  for (auto& msg : res.value()) {
    if (auto it = msg.ToJson(); it.has_value()) {
      it->removeMember("assistants");
      it->removeMember("title");
      msg_arr.append(it.value());
    } else {
      CTL_WRN("Failed to convert message to json: " + it.error());
    }
  }

  Json::Value root;
  root["object"] = "list";
  root["data"] = msg_arr;
  auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
  response->setStatusCode(k200OK);
  callback(response);
}

void Threads::CreateThread(
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

  // TODO: namh handle tool_resources
  // TODO: namh handle messages

  std::optional<Cortex::VariantMap> metadata = std::nullopt;
  if (json_body->get("metadata", "").isObject()) {
    auto res = Cortex::ConvertJsonValueToMap(json_body->get("metadata", ""));
    if (res.has_error()) {
      CTL_WRN("Failed to convert metadata to map: " + res.error());
    } else {
      metadata = res.value();
    }
  }

  auto res = thread_service_->CreateThread(nullptr, metadata);

  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    auto init_msg_res =
        message_service_->InitializeMessages(res->id, std::nullopt);

    if (res.has_error()) {
      CTL_ERR("Failed to convert message to json: " + res.error());
      Json::Value ret;
      ret["message"] = res.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
    } else {
      auto json_res = res->ToJson();
      json_res->removeMember("title");
      json_res->removeMember("assistants");
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(json_res.value());
      resp->setStatusCode(k200OK);
      callback(resp);
    }
  }
}

void Threads::RetrieveThread(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& thread_id) const {
	(void) req;
  auto res = thread_service_->RetrieveThread(thread_id);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    auto thread_to_json = res->ToJson();
    if (thread_to_json.has_error()) {
      CTL_ERR("Failed to convert message to json: " + thread_to_json.error());
      Json::Value ret;
      ret["message"] = thread_to_json.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
    } else {
      thread_to_json->removeMember("assistants");
      thread_to_json->removeMember("title");
      auto resp =
          cortex_utils::CreateCortexHttpJsonResponse(thread_to_json.value());
      resp->setStatusCode(k200OK);
      callback(resp);
    }
  }
}

void Threads::ModifyThread(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& thread_id) {
  auto json_body = req->getJsonObject();
  if (json_body == nullptr) {
    Json::Value ret;
    ret["message"] = "Request body can't be empty";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  std::optional<Cortex::VariantMap> metadata = std::nullopt;
  if (auto it = json_body->get("metadata", ""); it) {
    if (it.empty()) {
      Json::Value ret;
      ret["message"] = "Metadata can't be empty";
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }
    auto convert_res = Cortex::ConvertJsonValueToMap(it);
    if (convert_res.has_error()) {
      Json::Value ret;
      ret["message"] =
          "Failed to convert metadata to map: " + convert_res.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }
    metadata = convert_res.value();
  }

  if (!metadata.has_value()) {
    Json::Value ret;
    ret["message"] = "Metadata is mandatory";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // TODO: namh handle tools
  auto res =
      thread_service_->ModifyThread(thread_id, nullptr, metadata.value());
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    auto message_to_json = res->ToJson();
    if (message_to_json.has_error()) {
      CTL_ERR("Failed to convert message to json: " + message_to_json.error());
      Json::Value ret;
      ret["message"] = message_to_json.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
    } else {
      auto json_res = res->ToJson();
      json_res->removeMember("title");
      json_res->removeMember("assistants");
      auto resp =
          cortex_utils::CreateCortexHttpJsonResponse(json_res.value());
      resp->setStatusCode(k200OK);
      callback(resp);
    }
  }
}

void Threads::DeleteThread(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& thread_id) {
	(void) req;
  auto res = thread_service_->DeleteThread(thread_id);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  api_response::DeleteSuccessResponse response;
  response.id = thread_id;
  response.object = "thread.deleted";
  response.deleted = true;
  auto resp =
      cortex_utils::CreateCortexHttpJsonResponse(response.ToJson().value());
  resp->setStatusCode(k200OK);
  callback(resp);
}
