#include "swagger.h"
#include "cortex_openapi.h"
#include "utils/cortex_utils.h"

constexpr auto ScalarUi = R"(
<!doctype html>
<html>
  <head>
    <title>Cortex API Reference</title>
    <meta charset="utf-8" />
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1" />
  </head>
  <body>
    <!-- Need a Custom Header? Check out this example https://codepen.io/scalarorg/pen/VwOXqam -->
    <script
      id="api-reference"
      data-url="/openapi.json"></script>
    <script src="https://cdn.jsdelivr.net/npm/@scalar/api-reference"></script>
  </body>
</html>
)";

Json::Value SwaggerController::generateOpenAPISpec() {
  Json::Value root;
  Json::Reader reader;
  reader.parse(CortexOpenApi::GetOpenApiJson(), root);
  return root;
}

void SwaggerController::serveSwaggerUI(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) const {
  auto resp = cortex_utils::CreateCortexHttpResponse();
  resp->setBody(ScalarUi);
  resp->setContentTypeCode(drogon::CT_TEXT_HTML);
  callback(resp);
}

void SwaggerController::serveOpenAPISpec(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) const {
  Json::Value spec = generateOpenAPISpec();
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(spec);
  callback(resp);
}
