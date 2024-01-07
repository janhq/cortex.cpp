#include "nitro/controllers/health.h"
#include "nitro/utils/nitro_utils.h"

void health::asyncHandleHttpRequest(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  auto resp = nitro_utils::nitroHttpResponse();
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(CT_TEXT_HTML);
  resp->setBody("Nitro is alive!!!");
  callback(resp);
}
