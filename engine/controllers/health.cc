#include "health.h"
#include "utils/cortex_utils.h"

void health::asyncHandleHttpRequest(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
	(void) req;
  auto resp = cortex_utils::CreateCortexHttpResponse();
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(CT_TEXT_HTML);
  resp->setBody("cortex-cpp is alive!!!");
  callback(resp);
}
