#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include <climits>  // for PATH_MAX
#include "controllers/command_line_parser.h"
#include "cortex-common/cortexpythoni.h"
#include "utils/archive_utils.h"
#include "utils/cortex_utils.h"
#include "utils/dylib.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <libgen.h>  // for dirname()
#include <mach-o/dyld.h>
#include <sys/types.h>
#elif defined(__linux__)
#include <libgen.h>  // for dirname()
#include <sys/types.h>
#include <unistd.h>  // for readlink()
#elif defined(_WIN32)
#include <windows.h>
#undef max
#else
#error "Unsupported platform!"
#endif


void run_server(){
  // Create logs/ folder and setup log to file
      std::filesystem::create_directory(cortex_utils::logs_folder);
      trantor::AsyncFileLogger asyncFileLogger;
      asyncFileLogger.setFileName(cortex_utils::logs_base_name);
      asyncFileLogger.startLogging();
      trantor::Logger::setOutputFunction(
          [&](const char* msg, const uint64_t len) {
            asyncFileLogger.output(msg, len);
          },
          [&]() { asyncFileLogger.flush(); });
      asyncFileLogger.setFileSizeLimit(cortex_utils::log_file_size_limit);
      // Number of cortex.cpp threads
      // if (argc > 1) {
      //   thread_num = std::atoi(argv[1]);
      // }

      // // Check for host argument
      // if (argc > 2) {
      //   host = argv[2];
      // }

      // // Check for port argument
      // if (argc > 3) {
      //   port = std::atoi(argv[3]);  // Convert string argument to int
      // }
      int thread_num = 1;
      std::string host = "127.0.0.1";
      int port = 3928;

      int logical_cores = std::thread::hardware_concurrency();
      int drogon_thread_num = std::max(thread_num, logical_cores);
      // cortex_utils::nitro_logo();
#ifdef CORTEX_CPP_VERSION
      LOG_INFO << "cortex.cpp version: " << CORTEX_CPP_VERSION;
#else
      LOG_INFO << "cortex.cpp version: undefined";
#endif

      LOG_INFO << "Server started, listening at: " << host << ":" << port;
      LOG_INFO << "Please load your model";
      drogon::app().addListener(host, port);
      drogon::app().setThreadNum(drogon_thread_num);
      LOG_INFO << "Number of thread is:" << drogon::app().getThreadNum();

      drogon::app().run();
      // return 0;
}

void fork_process() {
#if defined(_WIN32) || defined(_WIN64)
  // Windows-specific code to create a new process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  // Create child process
  if (!CreateProcess(
          NULL,  // No module name (use command line)
          cortex_utils::GetCurrentPath() +
              "/cortex-cpp.exe --start-server",  // Command line (replace with your actual executable)
          NULL,                   // Process handle not inheritable
          NULL,                   // Thread handle not inheritable
          FALSE,                  // Set handle inheritance to FALSE
          0,                      // No creation flags
          NULL,                   // Use parent's environment block
          NULL,                   // Use parent's starting directory
          &si,                    // Pointer to STARTUPINFO structure
          &pi))                   // Pointer to PROCESS_INFORMATION structure
  {
    std::cout << "Could not start server: " << GetLastError() << std::endl;
  } else {
    std::cout << "Server started" << std::endl;
  }

#else
  // Unix-like system-specific code to fork a child process
  pid_t pid = fork();

  if (pid < 0) {
    // Fork failed
    std::cerr << "Could not start server: " << std::endl;
    return;
  } else if (pid == 0) {
    // Child process
    run_server();
  } else {
    // Parent process
    std::cout << "Server started" << std::endl;
  }
#endif
}

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

  if (argc > 1) {
    if (strcmp(argv[1], "--start-server") == 0) {
      run_server();
      return 0;
    } else {
      CommandLineParser clp;
      clp.SetupCommand(argc, argv);
      return 0;
    }
  }

  fork_process();
}
