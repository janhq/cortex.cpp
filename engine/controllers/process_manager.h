#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>
#include "services/engine_service.h"

using namespace drogon;

class ProcessManager : public drogon::HttpController<ProcessManager, false> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(ProcessManager::destroy, "/destroy", Options, Delete);
  METHOD_LIST_END

  void destroy(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& callback);

  ProcessManager(std::shared_ptr<EngineService> engine_service)
      : engine_service_(engine_service) {}

 private:
  std::shared_ptr<EngineService> engine_service_;
};
