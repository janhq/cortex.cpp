#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>
#include <string>

using namespace drogon;

namespace workers {

class pyrunner : public drogon::HttpController<pyrunner> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(pyrunner::executePythonFileRequest, "execute", Post);
  METHOD_LIST_END

 private:
  void executePythonFileRequest(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
};
}  // namespace workers
