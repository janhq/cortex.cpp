#include "pyrunner.h"
#include <dlfcn.h>
#include <unistd.h>
#include <cstdio>
#include "utils/nitro_utils.h"

workers::pyrunner::pyrunner() {
  LOG_INFO << "Looking for libpython inside lib";
  default_python_lib_dir = findPythonLib("lib");
  LOG_INFO << "Found Python in path: " + default_python_lib_dir;
}

workers::pyrunner::~pyrunner() {
  if (Py_Finalize != nullptr) {
    Py_Finalize();
  }
}

void workers::pyrunner::testrun(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {

  printf("Starting program...\n");

  void* libPython = dlopen(default_python_lib_dir.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (!libPython) {
    fprintf(stderr, "Failed to load Python library: %s\n", dlerror());
  }

  Py_Initialize = (Py_InitializeFunc)dlsym(libPython, "Py_Initialize");
  Py_Finalize = (Py_FinalizeFunc)dlsym(libPython, "Py_Finalize");
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

  // Py_Finalize();
  dlclose(libPython);

  Json::Value jsonResp;
  jsonResp["message"] = "Python test run succesfully done";
  auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(response);
  return;
};

void workers::pyrunner::PyRunPath(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {

  const auto& jsonBody = req->getJsonObject();

  if (!jsonBody) {
    Json::Value jsonResp;
    jsonResp["message"] = "Json body is empty";
    auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
    callback(response);
    return;
  }

  // Get python module folder path
  std::string PyModulePath = jsonBody->operator[]("py_module_path").asString();
  std::string PyEntryPoint = jsonBody->get("entrypoint", "main.py").asString();

  if (PyModulePath == "") {
    Json::Value jsonResp;
    jsonResp["message"] = "No specified PyModulePath";
    auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
    callback(response);
    return;
  }

  printf("Starting program...\n");


  auto libDeleter = [](void* lib) {
      if (lib) dlclose(lib);
  };
  std::unique_ptr<void, decltype(libDeleter)> libPython(dlopen(default_python_lib_dir.c_str(), RTLD_LAZY | RTLD_GLOBAL), libDeleter);
  if (!libPython) {
      fprintf(stderr, "Failed to load Python library: %s\n", dlerror());
      return;
  }
  Py_Initialize = reinterpret_cast<Py_InitializeFunc>(dlsym(libPython.get(), "Py_Initialize"));
  Py_Finalize = reinterpret_cast<Py_FinalizeFunc>(dlsym(libPython.get(), "Py_Finalize"));
  auto PyRun_SimpleString = reinterpret_cast<PyRun_SimpleStringFunc>(dlsym(libPython.get(), "PyRun_SimpleString"));
  auto PyErr_Print = reinterpret_cast<PyErr_PrintFunc>(dlsym(libPython.get(), "PyErr_Print"));
  auto PyRun_SimpleFile =
      (PyRun_SimpleFileFunc)dlsym(libPython.get(), "PyRun_SimpleFile");

  if (!Py_Initialize || !Py_Finalize || !PyRun_SimpleString || !PyErr_Print) {
      fprintf(stderr, "Failed to bind necessary Python functions\n");
      return;
  }


  pid_t pid = fork();

  if (pid == -1)
  {
    fprintf(stderr, "Failed to fork\n");
    return;
  }

  if (pid == 0)
  {
    Py_Initialize = reinterpret_cast<Py_InitializeFunc>(dlsym(libPython.get(), "Py_Initialize"));
    Py_Finalize = reinterpret_cast<Py_FinalizeFunc>(dlsym(libPython.get(), "Py_Finalize"));
    Py_Initialize();

    std::string fileEntry = PyModulePath + "/" + PyEntryPoint;

    LOG_INFO << "Trying to run file at: " << fileEntry;
    // After the PyRun_SimpleString call that sets up sys.path
    FILE* file = fopen(fileEntry.c_str(), "r");
    if (file == NULL) {
      fprintf(stderr, "Failed to open external_script.py\n");
    } else {
      LOG_INFO << "before run here";
      if (PyRun_SimpleFile(file, fileEntry.c_str() ) != 0) {
        PyErr_Print();
        fprintf(stderr, "Python script file execution failed.\n");
      }
      fclose(file);
    }

    Py_Finalize();

    Json::Value jsonResp;
    jsonResp["message"] = "Python test run succesfully done";
    auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
    callback(response);
    LOG_INFO << "Child process has finished.";
    return;
  }
};
// Add definition of your processing function here
