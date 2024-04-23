#pragma once

#include <dlfcn.h>
#include <trantor/utils/Logger.h>
#include <csignal>
#include <filesystem>
#include <regex>
#include <string>

#if defined(_WIN32)
  #define PY_LIB HMODULE
  #define PY_LOAD_LIB(path) LoadLibraryW(nitro_utils::stringToWString(path).c_str());
  #define GET_PY_FUNC GetProcAddress
  #define PY_FREE_LIB FreeLibrary
#else
  #define PY_LIB void*
  #define PY_LOAD_LIB(path) dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  #define GET_PY_FUNC dlsym
  #define PY_FREE_LIB dlclose
  extern char **environ; // Environment variable for posix_spawn
#endif

namespace python_embedding_helper {

typedef void (*Py_InitializeFunc)();
typedef void (*Py_FinalizeFunc)();
typedef void (*PyErr_PrintFunc)();
typedef int (*PyRun_SimpleStringFunc)(const char*);
typedef int (*PyRun_SimpleFileFunc)(FILE*, const char*);

inline void signalHandler(int signum) {
  std::cout << "Interrupt signal (" << signum << ") received.\n";
  abort();
}

inline std::string findPythonLib(const std::string& libDir) {
  std::string pattern;
#if defined(_WIN32) || defined(_WIN64)
  // Windows
  pattern = "python[0-9][0-9]+.*dll";
#elif defined(__APPLE__) || defined(__MACH__)
  // macOS
  pattern = "libpython[0-9]+\\.[0-9]+\\.dylib";
#else
  // Linux or other Unix-like systems
  pattern = "libpython[0-9]+\\.[0-9]+\\.so.*";
#endif
  std::regex regexPattern(pattern);
  for (const auto& entry : std::filesystem::directory_iterator(libDir)) {
    std::string fileName = entry.path().filename().string();
    if (std::regex_match(fileName, regexPattern)) {
      return entry.path().string();
    }
  }
  return "";  // Return an empty string if no matching library is found
}

inline void executePythonFile(std::string py_lib_path ,std::string py_file_path) {

  signal(SIGINT, signalHandler);

  std::string py_dl_path = findPythonLib(py_lib_path);
  if (py_dl_path == "") {
    LOG_ERROR << "Could not find Python dynamic library file int path: " << py_lib_path;
    return;
  }

  PY_LIB libPython = PY_LOAD_LIB(py_dl_path);
  if (!libPython) {
    LOG_ERROR << "Failed to load Python dynamic library from path: " << py_dl_path;
    return;
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


} // namespace python_embedding_helper
