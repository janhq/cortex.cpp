#pragma once

#include <drogon/HttpController.h>
#include <json/json.h>

using namespace drogon;

class SwaggerController
    : public drogon::HttpController<SwaggerController, false> {

  constexpr static auto ScalarUi = R"(
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

 public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SwaggerController::serveSwaggerUI, "/", Get);
  ADD_METHOD_TO(SwaggerController::serveOpenAPISpec, "/openapi.json", Get);
  METHOD_LIST_END

  explicit SwaggerController(const std::string& host, const std::string& port)
      : host_{host}, port_{port} {};

  void serveSwaggerUI(
      const drogon::HttpRequestPtr& req,
      std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;

  void serveOpenAPISpec(
      const drogon::HttpRequestPtr& req,
      std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;

 private:
  std::string host_;
  std::string port_;

  Json::Value GenerateOpenApiSpec() const;
};
