#include "pyrunner.h"
#include <dlfcn.h>
#include <csignal>
#include "utils/nitro_utils.h"

#if defined(__linux__)
  #include <spawn.h>
  #include <sys/wait.h>
  #define PY_LIB void*
  #define PY_LOAD_LIB(path) dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  #define GET_PY_FUNC dlsym
  #define PY_FREE_LIB dlclose
#elif defined(_WIN32)
  #include <process.h>
  #define PY_LIB HMODULE
  #define PY_LOAD_LIB(path) LoadLibraryW(nitro_utils::stringToWString(path).c_str());
  #define GET_PY_FUNC GetProcAddress
  #define PY_FREE_LIB FreeLibrary
#endif

void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  abort();
}

workers::pyrunner::pyrunner() {}
workers::pyrunner::~pyrunner() {}

void workers::pyrunner::executePythonFile(std::string py_dl_path ,std::string py_file_path) {

  signal(SIGINT, signalHandler);

  PY_LIB libPython = PY_LOAD_LIB(py_dl_path);
  if (!libPython) {
    LOG_ERROR << "Failed to load Python dynamic library from path: " << py_dl_path;
  } else {
    LOG_INFO << "Successully loaded Python dynamic library from path: " << py_dl_path;
  }

  auto Py_Initialize = (Py_InitializeFunc)GET_PY_FUNC(libPython, "Py_Initialize");
  auto Py_Finalize = (Py_FinalizeFunc)GET_PY_FUNC(libPython, "Py_Finalize");
  auto PyErr_Print = (PyErr_PrintFunc)GET_PY_FUNC(libPython, "PyErr_Print");
  auto PyRun_SimpleString = (PyRun_SimpleStringFunc)GET_PY_FUNC(libPython, "PyRun_SimpleString");
  auto PyRun_SimpleFile = (PyRun_SimpleFileFunc)GET_PY_FUNC(libPython, "PyRun_SimpleFile");

  if (!Py_Initialize || !Py_Finalize || !PyErr_Print || !PyRun_SimpleString || !PyRun_SimpleFile)
  {
    fprintf(stderr, "Failed to bind necessary Python functions\n");
    PY_FREE_LIB(libPython);
    return;
  }

  Py_Initialize();
  LOG_INFO << "Trying to run Python file in path " << py_file_path;
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
  PY_FREE_LIB(libPython);
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
    jsonResp["message"] = "No specified Python dynamic library path";
    auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
    callback(response);
    return;
  }

  if (py_file_path == "") {
  Json::Value jsonResp;
  jsonResp["message"] = "No specified Python file path";
  auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(response);
  return;
}

  Json::Value jsonResp;
  jsonResp["message"] = "Running the Python file";
  LOG_INFO << "Creating a new child process for Python embedding...\n";

#if defined(__linux__)
  std::string child_process_exe_path = nitro_utils::getCurrentExecutablePath();
  std::vector<char*> child_process_args;
  child_process_args.push_back(const_cast<char*>(child_process_exe_path.c_str()));
  child_process_args.push_back(const_cast<char*>("--run_python_file"));
  child_process_args.push_back(const_cast<char*>(py_dl_path.c_str()));
  child_process_args.push_back(const_cast<char*>(py_file_path.c_str()));

  pid_t pid;
  posix_spawnattr_t attr;
  posix_spawn_file_actions_t file_actions;
  posix_spawnattr_init(&attr);
  posix_spawn_file_actions_init(&file_actions);

  int status = posix_spawn(&pid, child_process_exe_path.c_str(), &file_actions, 
                           &attr, child_process_args.data(), environ);
  if (status) {
      LOG_ERROR << "Failed to spawn process: " << strerror(status);
      jsonResp["message"] = "Failed to execute the Python file";
  }

#elif defined(_WIN32)
  std::wstring exePath = nitro_utils::getCurrentExecutablePath();
  std::wstring pyArrs = nitro_utils::stringToWString(" --run_python_file " + py_dl_path + " " + py_file_path);

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcessW(const_cast<wchar_t*>(exePath.data()), // the path to the executable file
                      const_cast<wchar_t*>(pyArrs.data()), // command line arguments passed to the child
                      NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    LOG_ERROR << "Failed to create child process: " << GetLastError();
    jsonResp["message"] = "Failed to execute the Python file";
  }
#endif

  posix_spawn_file_actions_destroy(&file_actions);
  posix_spawnattr_destroy(&attr);

  auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(response);
  return;
};
// Add definition of your processing function here
