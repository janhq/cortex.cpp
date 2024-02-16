#include "prelight.h"

void prelight::handlePrelight(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setStatusCode(drogon::HttpStatusCode::k200OK);
  resp->addHeader("Access-Control-Allow-Origin", "*");
  resp->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  resp->addHeader("Access-Control-Allow-Headers", "*");
  callback(resp);
}

