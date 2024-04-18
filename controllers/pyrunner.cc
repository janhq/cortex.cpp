#include "pyrunner.h"
#include <dlfcn.h>
#include <csignal>
#include "utils/nitro_utils.h"

#if defined(__linux__)
  // #include <unistd.h>
#elif defined(_WIN32)
  #include <process.h>
#endif

void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  abort();
}

workers::pyrunner::pyrunner() {}
workers::pyrunner::~pyrunner() {}

void workers::pyrunner::executePythonFile(std::string py_dl_path ,std::string py_file_path) {

  signal(SIGINT, signalHandler);

  HMODULE libPython = LoadLibraryW(nitro_utils::stringToWString(py_dl_path).c_str());
  if (!libPython) {
    LOG_ERROR << "Failed to load python dynamic library from path: " << py_dl_path;
  } else {
    LOG_INFO << "Successully loaded python dynamic library from path: " << py_dl_path;
  }

  auto Py_Initialize = (Py_InitializeFunc)GetProcAddress(libPython, "Py_Initialize");
  auto Py_Finalize = (Py_FinalizeFunc)GetProcAddress(libPython, "Py_Finalize");
  auto PyErr_Print = (PyErr_PrintFunc)GetProcAddress(libPython, "PyErr_Print");
  auto PyRun_SimpleString = (PyRun_SimpleStringFunc)GetProcAddress(libPython, "PyRun_SimpleString");
  auto PyRun_SimpleFile = (PyRun_SimpleFileFunc)GetProcAddress(libPython, "PyRun_SimpleFile");

  if (!Py_Initialize || !Py_Finalize || !PyErr_Print || !PyRun_SimpleString || !PyRun_SimpleFile)
  {
    fprintf(stderr, "Failed to bind necessary Python functions testrunV2\n");
    FreeLibrary(libPython);
  }

  Py_Initialize();
  LOG_INFO << "Trying to run python file in path " << py_file_path;
  FILE* file = fopen(py_file_path.c_str(), "r");
  if (file == NULL) {
    fprintf(stderr, "Failed to open %s\n", py_file_path.c_str());
  } else {
    if (PyRun_SimpleFile(file, py_file_path.c_str() ) != 0) {
      PyErr_Print();
      fprintf(stderr, "Python script file execution failed.\n");
    }
    fclose(file);
  }

  Py_Finalize();
  FreeLibrary(libPython);
}

void workers::pyrunner::executePythonFileRequest(
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

  std::string py_dl_path = jsonBody->operator[]("py_dynamic_lib_path").asString();
  std::string py_file_path = jsonBody->operator[]("py_file_path").asString();

  if (py_dl_path == "") {
    Json::Value jsonResp;
    jsonResp["message"] = "No specified python dynamic library path";
    auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
    callback(response);
    return;
  }

  if (py_file_path == "") {
  Json::Value jsonResp;
  jsonResp["message"] = "No specified python file path";
  auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(response);
  return;
}

  printf("Creating a new child process for python embedding...\n");

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  std::wstring fileNameChild = nitro_utils::getCurrentExecutablePath();
  std::wstring pyEntryPointPath = nitro_utils::stringToWString(" --run_python_file " + py_dl_path + " " + py_file_path);

  Json::Value jsonResp;
  jsonResp["message"] = "Running the python file";

  if (!CreateProcessW(const_cast<wchar_t*>(fileNameChild.data()), // the path to the executable file
                      const_cast<wchar_t*>(pyEntryPointPath.data()), // command line arguments passed to the child
                      NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    LOG_ERROR << "Failed to create child process: " << GetLastError();
    jsonResp["message"] = "Failed to execute the python file";
  }

  auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(response);
  return;
};
// Add definition of your processing function here
