#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>

using namespace drogon;

class ProcessManager : public drogon::HttpController<ProcessManager, false> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(ProcessManager::destroy, "/destroy", Options, Delete);
  METHOD_LIST_END

  void destroy(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& callback);
};
