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
  METHOD_LIST_BEGIN

  ADD_METHOD_TO(pyrunner::testrun, "/testrun", Get);
  METHOD_ADD(pyrunner::PyRunPath, "runpath", Post);

  // Method declarations...
  METHOD_LIST_END

 private:
  std::string default_python_lib_dir;

  void testrun(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& callback);

  void PyRunPath(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback);

  void ExecutePythonCode(const std::string& PyModulePath,
                         const std::string& PyEntryPoint);
  std::string findPythonLib(const std::string& libDir) {
    std::string pattern;
#if defined(_WIN32) || defined(_WIN64)
    // Windows
    pattern = "libpython[0-9]+\\.[0-9]+\\.dll";
#elif defined(__APPLE__) || defined(__MACH__)
    // macOS
    pattern = "libpython[0-9]+\\.[0-9]+\\.dylib";
#else
    // Linux or other Unix-like systems
    pattern = "libpython[0-9]+\\.[0-9]+\\.so.*";
#endif
    std::regex regexPattern(pattern);
    for (const auto& entry : fs::directory_iterator(libDir)) {
      std::string fileName = entry.path().filename().string();
      if (std::regex_match(fileName, regexPattern)) {
        return entry.path().string();
      }
    }
    return "";  // Return an empty string if no matching library is found
  }
};
}  // namespace workers
