#pragma once

#include <drogon/HttpController.h>
#include "utils/cortex_utils.h"

using namespace drogon;

namespace http_util {

inline bool HasFieldInReq(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>& callback,
                          const std::string& field) {
  if (auto o = req->getJsonObject(); !o || (*o)[field].isNull()) {
    Json::Value res;
    res["message"] = "No " + field + " field in request body";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "No " << field << " field in request body";
    return false;
  }
  return true;
}

}  // namespace http_util
