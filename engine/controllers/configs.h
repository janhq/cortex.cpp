#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpRequest.h>
#include "services/config_service.h"

using namespace drogon;

class Configs : public drogon::HttpController<Configs, false> {
 public:
  METHOD_LIST_BEGIN

  METHOD_ADD(Configs::GetConfigurations, "", Get);
  METHOD_ADD(Configs::UpdateConfigurations, "", Patch);

  ADD_METHOD_TO(Configs::GetConfigurations, "/v1/configs", Get);
  ADD_METHOD_TO(Configs::UpdateConfigurations, "/v1/configs", Patch);

  METHOD_LIST_END

  explicit Configs(std::shared_ptr<ConfigService> config_service)
      : config_service_{config_service} {}

  void GetConfigurations(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) const;

  void UpdateConfigurations(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback);

 private:
  std::shared_ptr<ConfigService> config_service_;
};
