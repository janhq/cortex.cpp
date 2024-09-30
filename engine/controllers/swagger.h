#pragma once

#include <drogon/HttpController.h>
#include <json/json.h>

using namespace drogon;

class SwaggerController : public drogon::HttpController<SwaggerController> {
 public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SwaggerController::serveSwaggerUI, "/swagger", Get);
  ADD_METHOD_TO(SwaggerController::serveOpenAPISpec, "/openapi.json", Get);
  METHOD_LIST_END

  void serveSwaggerUI(
      const drogon::HttpRequestPtr& req,
      std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;

  void serveOpenAPISpec(
      const drogon::HttpRequestPtr& req,
      std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;

 private:
  static const std::string swaggerUIHTML;
  static Json::Value generateOpenAPISpec();
};