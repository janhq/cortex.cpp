#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include <climits>  // for PATH_MAX
#include <iostream>
#include "cortex-common/cortexpythoni.h"
#include "utils/cortex_utils.h"
#include "utils/dylib.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <libgen.h>  // for dirname()
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <libgen.h>  // for dirname()
#include <unistd.h>  // for readlink()
#elif defined(_WIN32)
#include <windows.h>
#undef max
#else
#error "Unsupported platform!"
#endif

int main(int argc, char* argv[]) {
  // Check if this process is for python execution
  if (argc > 1) {
    if (strcmp(argv[1], "--run_python_file") == 0) {
      std::string py_home_path = (argc > 3) ? argv[3] : "";
      std::unique_ptr<cortex_cpp::dylib> dl;
      try {
        std::string abs_path = cortex_utils::GetCurrentPath() +
                               cortex_utils::kPythonRuntimeLibPath;
        dl = std::make_unique<cortex_cpp::dylib>(abs_path, "engine");
      } catch (const cortex_cpp::dylib::load_error& e) {
        LOG_ERROR << "Could not load engine: " << e.what();
        return 1;
      }

      auto func = dl->get_function<CortexPythonEngineI*()>("get_engine");
      auto e = func();
      e->ExecutePythonFile(argv[0], argv[2], py_home_path);
      return 0;
    }
  }

  int thread_num = 1;
  std::string host = "127.0.0.1";
  int port = 3928;

  // Number of cortex-cpp threads
  if (argc > 1) {
    thread_num = std::atoi(argv[1]);
  }

  // Check for host argument
  if (argc > 2) {
    host = argv[2];
  }

  // Check for port argument
  if (argc > 3) {
    port = std::atoi(argv[3]);  // Convert string argument to int
  }

  int logical_cores = std::thread::hardware_concurrency();
  int drogon_thread_num = std::max(thread_num, logical_cores);
  // cortex_utils::nitro_logo();
#ifdef CORTEX_CPP_VERSION
  LOG_INFO << "cortex-cpp version: " << CORTEX_CPP_VERSION;
#else
  LOG_INFO << "cortex-cpp version: undefined";
#endif

  LOG_INFO << "Server started, listening at: " << host << ":" << port;
  LOG_INFO << "Please load your model";
  drogon::app().addListener(host, port);
  drogon::app().setThreadNum(drogon_thread_num);
  LOG_INFO << "Number of thread is:" << drogon::app().getThreadNum();

  drogon::app().run();

  return 0;
}
