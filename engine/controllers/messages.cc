#include "messages.h"
#include "common/api-dto/delete_success_response.h"
#include "common/message_content.h"
#include "common/message_role.h"
#include "common/variant_map.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

void Messages::ListMessages(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& thread_id, std::optional<std::string> limit,
    std::optional<std::string> order, std::optional<std::string> after,
    std::optional<std::string> before,
    std::optional<std::string> run_id) const {
  (void)req;
  auto res = message_service_->ListMessages(
      thread_id, std::stoi(limit.value_or("20")), order.value_or("desc"),
      after.value_or(""), before.value_or(""), run_id.value_or(""));

  Json::Value root;
  if (res.has_error()) {
    root["message"] = res.error();
    auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
    response->setStatusCode(k400BadRequest);
    callback(response);
    return;
  }
  Json::Value msg_arr(Json::arrayValue);
  for (auto& msg : res.value()) {
    if (auto it = msg.ToJson(); it.has_value()) {
      msg_arr.append(it.value());
    } else {
      CTL_WRN("Failed to convert message to json: " + it.error());
    }
  }

  root["object"] = "list";
  root["data"] = msg_arr;
  auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
  response->setStatusCode(k200OK);
  callback(response);
}

void Messages::CreateMessage(
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

  // role
  auto role_str = json_body->get("role", "").asString();
  if (role_str.empty()) {
    Json::Value ret;
    ret["message"] = "Role is required";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }
  if (role_str != "user" && role_str != "assistant") {
    Json::Value ret;
    ret["message"] = "Role must be either 'user' or 'assistant'";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto role = role_str == "user" ? OpenAi::Role::USER : OpenAi::Role::ASSISTANT;

  std::variant<std::string, std::vector<std::unique_ptr<OpenAi::Content>>>
      content;

  if (json_body->get("content", "").isArray()) {
    auto result = OpenAi::ParseContents(json_body->get("content", ""));
    if (result.has_error()) {
      Json::Value ret;
      ret["message"] = "Failed to parse content array: " + result.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }

    if (result.value().empty()) {
      Json::Value ret;
      ret["message"] = "Content array cannot be empty";
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }

    content = std::move(result.value());
  } else if (json_body->get("content", "").isString()) {
    auto content_str = json_body->get("content", "").asString();
    string_utils::Trim(content_str);
    if (content_str.empty()) {
      Json::Value ret;
      ret["message"] = "Content can't be empty";
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }

    // success get content as string
    content = content_str;
  } else {
    Json::Value ret;
    ret["message"] = "Content must be either a string or an array";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // attachments
  std::optional<std::vector<OpenAi::Attachment>> attachments = std::nullopt;
  if (json_body->get("attachments", "").isArray()) {
    attachments =
        OpenAi::ParseAttachments(std::move(json_body->get("attachments", "")))
            .value();
  }

  std::optional<Cortex::VariantMap> metadata = std::nullopt;
  if (json_body->get("metadata", "").isObject()) {
    auto res = Cortex::ConvertJsonValueToMap(json_body->get("metadata", ""));
    if (res.has_error()) {
      CTL_WRN("Failed to convert metadata to map: " + res.error());
    } else {
      metadata = res.value();
    }
  }

  auto res = message_service_->CreateMessage(
      thread_id, role, std::move(content), attachments, metadata);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = "Content must be either a string or an array";
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
      auto resp =
          cortex_utils::CreateCortexHttpJsonResponse(res->ToJson().value());
      resp->setStatusCode(k200OK);
      callback(resp);
    }
  }
}

void Messages::RetrieveMessage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& thread_id, const std::string& message_id) const {
  (void)req;
  auto res = message_service_->RetrieveMessage(thread_id, message_id);
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
      auto resp =
          cortex_utils::CreateCortexHttpJsonResponse(res->ToJson().value());
      resp->setStatusCode(k200OK);
      callback(resp);
    }
  }
}

void Messages::ModifyMessage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& thread_id, const std::string& message_id) {
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
  if (json_body->isMember("metadata")) {
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
  }

  std::optional<
      std::variant<std::string, std::vector<std::unique_ptr<OpenAi::Content>>>>
      content = std::nullopt;

  if (json_body->get("content", "").isArray()) {
    auto result = OpenAi::ParseContents(json_body->get("content", ""));
    if (result.has_error()) {
      Json::Value ret;
      ret["message"] = "Failed to parse content array: " + result.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }

    if (result.value().empty()) {
      Json::Value ret;
      ret["message"] = "Content array cannot be empty";
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }

    content = std::move(result.value());
  } else if (json_body->get("content", "").isString()) {
    auto content_str = json_body->get("content", "").asString();
    string_utils::Trim(content_str);
    if (content_str.empty()) {
      Json::Value ret;
      ret["message"] = "Content can't be empty";
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }

    content = content_str;
  } else if (!json_body->get("content", "").empty()) {
    Json::Value ret;
    ret["message"] = "Content must be either a string or an array";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  if (!metadata.has_value() && !content.has_value()) {
    Json::Value ret;
    ret["message"] = "Nothing to update";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto res = message_service_->ModifyMessage(thread_id, message_id, metadata,
                                             std::move(content));
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = "Failed to modify message: " + res.error();
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
      auto resp =
          cortex_utils::CreateCortexHttpJsonResponse(res->ToJson().value());
      resp->setStatusCode(k200OK);
      callback(resp);
    }
  }
}

void Messages::DeleteMessage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& thread_id, const std::string& message_id) {
  (void)req;
  auto res = message_service_->DeleteMessage(thread_id, message_id);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = "Failed to delete message: " + res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  api_response::DeleteSuccessResponse response;
  response.id = message_id;
  response.object = "thread.message.deleted";
  response.deleted = true;
  auto resp =
      cortex_utils::CreateCortexHttpJsonResponse(response.ToJson().value());
  resp->setStatusCode(k200OK);
  callback(resp);
}
