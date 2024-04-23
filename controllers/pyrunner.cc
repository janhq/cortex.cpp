#include "pyrunner.h"
#include <dlfcn.h>
#include "utils/nitro_utils.h"

#if defined(_WIN32)
  #include <process.h>
#else
  #include <spawn.h>
  extern char **environ; // Environment variable for posix_spawn
#endif


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

  std::string py_lib_path = jsonBody->operator[]("py_lib_path").asString();
  std::string py_file_path = jsonBody->operator[]("py_file_path").asString();

  if (py_lib_path == "") {
    LOG_WARN << "No specified Python library path, using default Python library from Nitro";
    py_lib_path = "./lib";
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

#if defined(_WIN32)
  std::wstring exePath = nitro_utils::getCurrentExecutablePath();
  std::wstring pyArrs = nitro_utils::stringToWString(" --run_python_file " + py_lib_path + " " + py_file_path);

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
  } else {
    LOG_INFO << "Created child process for Python embedding";
  }
#else
  std::string child_process_exe_path = nitro_utils::getCurrentExecutablePath();
  std::vector<char*> child_process_args;
  child_process_args.push_back(const_cast<char*>(child_process_exe_path.c_str()));
  child_process_args.push_back(const_cast<char*>("--run_python_file"));
  child_process_args.push_back(const_cast<char*>(py_lib_path.c_str()));
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
  } else {
    LOG_INFO << "Created child process for Python embedding";
  }
  posix_spawn_file_actions_destroy(&file_actions);
  posix_spawnattr_destroy(&attr);
#endif

  auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(response);
  return;
};
// Add definition of your processing function here
