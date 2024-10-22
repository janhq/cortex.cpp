#include "swagger.h"
#include "cortex_openapi.h"

constexpr auto ScalarUi = R"(
<!doctype html>
<html>
  <head>
    <title>Scalar API Reference</title>
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
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setBody(ScalarUi);
  resp->setContentTypeCode(drogon::CT_TEXT_HTML);
  callback(resp);
}

void SwaggerController::serveOpenAPISpec(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) const {
  Json::Value spec = generateOpenAPISpec();
  auto resp = drogon::HttpResponse::newHttpJsonResponse(spec);
  callback(resp);
}
