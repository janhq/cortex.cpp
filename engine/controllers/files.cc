#include "files.h"
#include "common/api-dto/delete_success_response.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

void Files::UploadFile(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  MultiPartParser parser;
  if (parser.parse(req) != 0 || parser.getFiles().size() != 1) {
    Json::Value root;
    root["message"] = "Must only be one file";
    auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
    response->setStatusCode(k400BadRequest);
    callback(response);
    return;
  }

  auto params = parser.getParameters();
  if (params.find("purpose") == params.end()) {
    Json::Value root;
    root["message"] = "purpose is mandatory";
    auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
    response->setStatusCode(k400BadRequest);
    callback(response);
    return;
  }

  auto purpose = params["purpose"];
  if (std::find(file_service_->kSupportedPurposes.begin(),
                file_service_->kSupportedPurposes.end(),
                purpose) == file_service_->kSupportedPurposes.end()) {
    Json::Value root;
    root["message"] =
        "purpose is not supported. Purpose can only one of these types: "
        "assistants, vision, batch or fine-tune";
    auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
    response->setStatusCode(k400BadRequest);
    callback(response);
    return;
  }

  const auto& file = parser.getFiles()[0];
  auto result =
      file_service_->UploadFile(file.getFileName(), purpose,
                                file.fileContent().data(), file.fileLength());

  if (result.has_error()) {
    Json::Value ret;
    ret["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    auto resp =
        cortex_utils::CreateCortexHttpJsonResponse(result->ToJson().value());
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Files::ListFiles(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback,
                      std::optional<std::string> purpose,
                      std::optional<std::string> limit,
                      std::optional<std::string> order,
                      std::optional<std::string> after) const {
                        (void) req;
  auto res = file_service_->ListFiles(
      purpose.value_or(""), std::stoi(limit.value_or("20")),
      order.value_or("desc"), after.value_or(""));
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

void Files::RetrieveFile(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& file_id,
                         std::optional<std::string> thread_id) const {
                          (void) req;
  // this code part is for backward compatible. remove it later on
  if (thread_id.has_value()) {
    auto msg_res =
        message_service_->RetrieveMessage(thread_id.value(), file_id);
    if (msg_res.has_error()) {
      Json::Value ret;
      ret["message"] = msg_res.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }

    if (msg_res->attachments->empty()) {
      auto res = file_service_->RetrieveFile(file_id);
      if (res.has_error()) {
        Json::Value ret;
        ret["message"] = res.error();
        auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
      }

      auto resp =
          cortex_utils::CreateCortexHttpJsonResponse(res->ToJson().value());
      resp->setStatusCode(k200OK);
      callback(resp);
      return;
    } else {
      if (!msg_res->attach_filename.has_value() || !msg_res->size.has_value()) {
        Json::Value ret;
        ret["message"] = "File not found or had been removed!";
        auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
        resp->setStatusCode(k404NotFound);
        callback(resp);
        return;
      }

      Json::Value ret;
      ret["object"] = "file";
      ret["created_at"] = msg_res->created_at;
      ret["filename"] = msg_res->attach_filename.value();
      ret["bytes"] = msg_res->size.value();
      ret["id"] = msg_res->id;
      ret["purpose"] = "assistants";

      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k200OK);
      callback(resp);
      return;
    }
  }

  auto res = file_service_->RetrieveFile(file_id);
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

void Files::DeleteFile(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& file_id) {
                        (void) req;
  auto res = file_service_->DeleteFileLocal(file_id);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  api_response::DeleteSuccessResponse response;
  response.id = file_id;
  response.object = "file";
  response.deleted = true;
  auto resp =
      cortex_utils::CreateCortexHttpJsonResponse(response.ToJson().value());
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Files::RetrieveFileContent(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& file_id, std::optional<std::string> thread_id) {
      (void) req;
  if (thread_id.has_value()) {
    auto msg_res =
        message_service_->RetrieveMessage(thread_id.value(), file_id);
    if (msg_res.has_error()) {
      Json::Value ret;
      ret["message"] = msg_res.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }

    if (msg_res->attachments->empty()) {
      auto res = file_service_->RetrieveFileContent(file_id);
      if (res.has_error()) {
        Json::Value ret;
        ret["message"] = res.error();
        auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
      }

      auto resp =
          cortex_utils::CreateCortexContentResponse(std::move(res.value()));
      callback(resp);
    } else {
      if (!msg_res->rel_path.has_value()) {
        Json::Value ret;
        ret["message"] = "File not found or had been removed";
        auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
      }

      auto content_res =
          file_service_->RetrieveFileContentByPath(msg_res->rel_path.value());

      if (content_res.has_error()) {
        Json::Value ret;
        ret["message"] = content_res.error();
        auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
      }

      auto resp = cortex_utils::CreateCortexContentResponse(
          std::move(content_res.value()));
      callback(resp);
    }
  }

  auto res = file_service_->RetrieveFileContent(file_id);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto resp = cortex_utils::CreateCortexContentResponse(std::move(res.value()));
  callback(resp);
}
