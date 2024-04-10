#include "pyrunner.h"
#include <dlfcn.h>
#include "utils/nitro_utils.h"

void pyrunner::testrun(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {

  printf("Starting program...\n");

  void* libPython = dlopen(default_python_lib_dir.c_str(), RTLD_LAZY);
  if (!libPython) {
    fprintf(stderr, "Failed to load Python library: %s\n", dlerror());
  }

  auto Py_Initialize = (Py_InitializeFunc)dlsym(libPython, "Py_Initialize");
  auto Py_Finalize = (Py_FinalizeFunc)dlsym(libPython, "Py_Finalize");
  auto PyErr_Print = (PyErr_PrintFunc)dlsym(libPython, "PyErr_Print");

  auto PyRun_SimpleString =
      (PyRun_SimpleStringFunc)dlsym(libPython, "PyRun_SimpleString");
  auto PyRun_SimpleFile =
      (PyRun_SimpleFileFunc)dlsym(libPython, "PyRun_SimpleFile");

  if (!Py_Initialize || !Py_Finalize || !PyRun_SimpleString ||
      !PyRun_SimpleFile) {
    fprintf(stderr, "Failed to bind necessary Python functions\n");
    dlclose(libPython);
  }

  Py_Initialize();

  PyRun_SimpleString("print('hello world')");

  Py_Finalize();
  dlclose(libPython);

  Json::Value jsonResp;
  jsonResp["message"] = "Python test run succesfully done";
  auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(response);
  return;
};
// Add definition of your processing function here
