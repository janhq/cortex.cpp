#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>
#include <filesystem>
#include <regex>
#include <string>

using namespace drogon;
namespace fs = std::filesystem;

typedef void (*Py_InitializeFunc)();
typedef void (*Py_FinalizeFunc)();
typedef void (*PyErr_PrintFunc)();
typedef int (*PyRun_SimpleStringFunc)(const char*);
typedef int (*PyRun_SimpleFileFunc)(FILE*, const char*);

namespace workers {

class pyrunner : public drogon::HttpController<pyrunner> {
 public:
  pyrunner();
  ~pyrunner();
  void executePythonFile(std::string py_dl_path ,std::string py_file_path);

  METHOD_LIST_BEGIN
  METHOD_ADD(pyrunner::executePythonFileRequest, "execute", Post);
  METHOD_LIST_END

 private:
  void executePythonFileRequest(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
};
}  // namespace workers
