#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>

using namespace drogon;

class processManager : public drogon::HttpController<processManager> {
public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  // METHOD_ADD(processManager::get, "/{2}/{1}", Get); // path is
  // /processManager/{arg2}/{arg1}
  METHOD_ADD(processManager::destroy, "/destroy",
             Delete); // path is /processManager/{arg1}/{arg2}/list

  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void destroy(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback);
  // void your_method_name(const HttpRequestPtr& req, std::function<void (const
  // HttpResponsePtr &)> &&callback, double p1, int p2) const;
};
