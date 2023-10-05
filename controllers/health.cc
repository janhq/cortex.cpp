#include "health.h"

void health::asyncHandleHttpRequest(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  // write your application logic here
  auto resp = HttpResponse::newHttpResponse();
  // NOTE: The enum constant below is named "k200OK" (as in 200 OK), not
  // "k2000K".
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(CT_TEXT_HTML);
  resp->setBody("Nitro is alive!!!");
  callback(resp);
  // write your application logic here
}
