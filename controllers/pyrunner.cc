#include "pyrunner.h"
#include <dlfcn.h>
#include <cstdio>
#include <csignal>
#include "utils/nitro_utils.h"
#include <iostream>
#include <vector>
#include <limits.h>

#if defined(__linux__)
  #include <sys/wait.h>
  #include <unistd.h>
#elif defined(_WIN32)
  #include <windows.h>
  #include <process.h>
#endif

void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  abort();
}

std::wstring stringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::wstring getCurrentExecutablePath() {
#if defined(__linux__)
    std::vector<char> buf(PATH_MAX);
    ssize_t len = readlink("/proc/self/exe", &buf[0], buf.size());
    if (len == -1 || len == buf.size()) {
        std::cerr << "Error reading symlink /proc/self/exe." << std::endl;
        return "";
    }
    return std::string(&buf[0], len);

#elif defined(_WIN32)
    wchar_t path[MAX_PATH];  // Use wide character array
    DWORD result = GetModuleFileNameW(NULL, path, MAX_PATH);  // Use the wide-character version of the function
    if (result == 0) {
        std::wcerr << L"Error getting module file name." << std::endl;
        return L"";
    }
    return std::wstring(path);
#endif
}

workers::pyrunner::pyrunner() {
  signal(SIGINT, signalHandler);
}

workers::pyrunner::~pyrunner() {}

void workers::pyrunner::runPythonFile(std::string pyHomePath ,std::string pyFileName) {

  LOG_INFO << "Looking for python dll file in " << pyHomePath;
  default_python_lib_dir = findPythonLib(pyHomePath);
  LOG_INFO << "Found Python in path: " + default_python_lib_dir;
  std::wstring converted_path = stringToWString(default_python_lib_dir);
  HMODULE libPython = LoadLibraryW(converted_path.c_str());
  if (!libPython) {
    fprintf(stderr, "Failed to load Python library: %s\n", dlerror());
  }

  auto Py_Initialize = (Py_InitializeFunc)GetProcAddress(libPython, "Py_Initialize");
  auto Py_Finalize = (Py_FinalizeFunc)GetProcAddress(libPython, "Py_Finalize");
  auto PyErr_Print = (PyErr_PrintFunc)GetProcAddress(libPython, "PyErr_Print");
  auto PyRun_SimpleString = (PyRun_SimpleStringFunc)GetProcAddress(libPython, "PyRun_SimpleString");
  auto PyRun_SimpleFile = (PyRun_SimpleFileFunc)GetProcAddress(libPython, "PyRun_SimpleFile");

  if (!Py_Initialize || !Py_Finalize || !PyErr_Print || !PyRun_SimpleString || !PyRun_SimpleFile)
  {
    fprintf(stderr, "Failed to bind necessary Python functions testrunV2\n");
    // dlclose(libPython);
    // FreeLibrary(libPython);
  }

  LOG_INFO << "CAMERON 2";
  Py_Initialize();
  LOG_INFO << "CAMERON 3";

  LOG_INFO << "Trying to run file at: " << pyFileName;
  // After the PyRun_SimpleString call that sets up sys.path
  FILE* file = fopen(pyFileName.c_str(), "r");
  if (file == NULL) {
    fprintf(stderr, "Failed to open %s\n", pyFileName.c_str());
  } else {
    LOG_INFO << "before run here";
    if (PyRun_SimpleFile(file, pyFileName.c_str() ) != 0) {
      PyErr_Print();
      fprintf(stderr, "Python script file execution failed.\n");
    }
    fclose(file);
  }

  Py_Finalize();
  FreeLibrary(libPython);
}

void workers::pyrunner::testrun(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {

  printf("Starting program...\n");

  void* libPython; // = open(default_python_lib_dir.c_str(), RTLD_LAZY | RTLD_GLOBAL);
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
    fprintf(stderr, "Failed to bind necessary Python functions in testrun\n");
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

  // auto libDeleter = [](void* lib) {
  //     if (lib) dlclose(lib);
  // };
  // std::unique_ptr<void, decltype(libDeleter)> libPython(dlopen(default_python_lib_dir.c_str(), RTLD_LAZY | RTLD_GLOBAL), libDeleter);
  // if (!libPython) {
  //     fprintf(stderr, "Failed to load Python library: %s\n", dlerror());
  //     return;
  // }
  // auto Py_Initialize = reinterpret_cast<Py_InitializeFunc>(dlsym(libPython.get(), "Py_Initialize"));
  // auto Py_Finalize = reinterpret_cast<Py_FinalizeFunc>(dlsym(libPython.get(), "Py_Finalize"));
  // auto PyRun_SimpleString = reinterpret_cast<PyRun_SimpleStringFunc>(dlsym(libPython.get(), "PyRun_SimpleString"));
  // auto PyErr_Print = reinterpret_cast<PyErr_PrintFunc>(dlsym(libPython.get(), "PyErr_Print"));
  // auto PyRun_SimpleFile =
  //     (PyRun_SimpleFileFunc)dlsym(libPython.get(), "PyRun_SimpleFile");

  // if (!Py_Initialize || !Py_Finalize || !PyRun_SimpleString || !PyErr_Print) {
  //     fprintf(stderr, "Failed to bind necessary Python functions\n");
  //     return;
  // }

  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));


  std::wstring fileNameChild = getCurrentExecutablePath();
  std::wstring pyEntryPointPath = stringToWString(" " + PyModulePath + " " + PyEntryPoint);

  if (!CreateProcessW( const_cast<wchar_t*>(fileNameChild.data()), // the path to the executable file
                       const_cast<wchar_t*>(pyEntryPointPath.data()), // command line arguments passed to the child
                       NULL,    // process security attributes
                       NULL,    // primary thread security attributes
                       FALSE,   // handles are inherited
                       0,       // creation flags
                       NULL,    // use parent's environment
                       NULL,    // use parent's current directory
                       &si,     // STARTUPINFO pointer
                       &pi))    // receives PROCESS_INFORMATION
    {
        std::cerr << "CreateProcess failed (" << GetLastError() << ")." << std::endl;
    }

  // pid_t pid = fork();

  // if (pid == -1)
  // {
  //   fprintf(stderr, "Failed to fork process for Python runtime\n");
  //   return;
  // }

  // if (pid == 0)
  // {
  //   drogon::app().quit();
  //   Py_Initialize();

  //   std::string fileEntry = PyModulePath + "/" + PyEntryPoint;

  //   LOG_INFO << "Trying to run file at: " << fileEntry;
  //   // After the PyRun_SimpleString call that sets up sys.path
  //   FILE* file = fopen(fileEntry.c_str(), "r");
  //   if (file == NULL) {
  //     fprintf(stderr, "Failed to open %s\n", fileEntry.c_str());
  //   } else {
  //     LOG_INFO << "before run here";
  //     if (PyRun_SimpleFile(file, fileEntry.c_str() ) != 0) {
  //       PyErr_Print();
  //       fprintf(stderr, "Python script file execution failed.\n");
  //     }
  //     fclose(file);
  //   }

  //   Py_Finalize();
  //   LOG_INFO << "Child process has finished.";
  //   abort();
  // } 

  Json::Value jsonResp;
  jsonResp["message"] = "Running Python code";
  auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(response);
  return;
};
// Add definition of your processing function here
