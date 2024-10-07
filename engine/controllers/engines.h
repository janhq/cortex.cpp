#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpRequest.h>
#include <trantor/utils/Logger.h>
#include "services/engine_service.h"

using namespace drogon;

class Engines : public drogon::HttpController<Engines> {
 public:
  Engines() : engine_service_{EngineService()} {};

  METHOD_LIST_BEGIN
  METHOD_ADD(Engines::InstallEngine, "/install/{1}", Post);
  METHOD_ADD(Engines::UninstallEngine, "/{1}", Delete);
  METHOD_ADD(Engines::ListEngine, "", Get);
  METHOD_ADD(Engines::GetEngine, "/{1}", Get);

  ADD_METHOD_TO(Engines::InstallEngine, "/v1/engines/install/{1}", Post);
  ADD_METHOD_TO(Engines::UninstallEngine, "/v1/engines/{1}", Delete);
  ADD_METHOD_TO(Engines::ListEngine, "/v1/engines", Get);
  ADD_METHOD_TO(Engines::GetEngine, "/v1/engines/{1}", Get);
  METHOD_LIST_END

  void InstallEngine(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& engine);

  void ListEngine(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback) const;

  void GetEngine(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback,
                 const std::string& engine) const;

  void UninstallEngine(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& engine);

 private:
  EngineService engine_service_;
};
