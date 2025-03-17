#include "swagger.h"
#include "cortex_openapi.h"
#include "utils/cortex_utils.h"

Json::Value SwaggerController::GenerateOpenApiSpec() const {
  Json::Value root;
  Json::Reader reader;
  reader.parse(CortexOpenApi::GetOpenApiJson(), root);

  Json::Value server_url;
  server_url["url"] = "http://" + host_ + ":" + port_;
  Json::Value resp_data(Json::arrayValue);
  resp_data.append(server_url);

  root["servers"] = resp_data;
  return root;
}

void SwaggerController::serveSwaggerUI(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) const {
	(void) req;
  auto resp = cortex_utils::CreateCortexHttpResponse();
  resp->setBody(ScalarUi);
  resp->setContentTypeCode(drogon::CT_TEXT_HTML);
  callback(resp);
}

void SwaggerController::serveOpenAPISpec(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) const {
	(void) req;
  auto spec = GenerateOpenApiSpec();
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(spec);
  callback(resp);
}
