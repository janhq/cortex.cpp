#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>

using namespace drogon;

class processManager : public drogon::HttpController<processManager> {
public:
  METHOD_LIST_BEGIN

  METHOD_ADD(processManager::destroy, "/destroy",
             Delete); // path is /processManager/{arg1}/{arg2}/list

  METHOD_LIST_END

  void destroy(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback);
};
