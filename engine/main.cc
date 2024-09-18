#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include <climits>  // for PATH_MAX
#include "commands/cortex_upd_cmd.h"
#include "controllers/command_line_parser.h"
#include "cortex-common/cortexpythoni.h"
#include "utils/archive_utils.h"
#include "utils/cortex_utils.h"
#include "utils/dylib.h"
#include "utils/file_logger.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/system_info_utils.h"

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

void RunServer() {
  auto config = file_manager_utils::GetCortexConfig();
  LOG_INFO << "Host: " << config.apiServerHost
           << " Port: " << config.apiServerPort << "\n";

  // Create logs/ folder and setup log to file
  std::filesystem::create_directories(
      std::filesystem::path(config.logFolderPath) /
      std::filesystem::path(cortex_utils::logs_folder));
  trantor::FileLogger asyncFileLogger;
  asyncFileLogger.setFileName(config.logFolderPath + "/" +
                              cortex_utils::logs_base_name);
  asyncFileLogger.setMaxLines(config.maxLogLines);  // Keep last 100000 lines
  asyncFileLogger.startLogging();
  trantor::Logger::setOutputFunction(
      [&](const char* msg, const uint64_t len) {
        asyncFileLogger.output_(msg, len);
      },
      [&]() { asyncFileLogger.flush(); });
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
  // std::string host = "127.0.0.1";
  // int port = 3928;

  int logical_cores = std::thread::hardware_concurrency();
  int drogon_thread_num = std::max(thread_num, logical_cores);
  // cortex_utils::nitro_logo();
#ifdef CORTEX_CPP_VERSION
  LOG_INFO << "cortex.cpp version: " << CORTEX_CPP_VERSION;
#else
  LOG_INFO << "cortex.cpp version: undefined";
#endif

  LOG_INFO << "Server started, listening at: " << config.apiServerHost << ":"
           << config.apiServerPort;
  LOG_INFO << "Please load your model";
  drogon::app().addListener(config.apiServerHost,
                            std::stoi(config.apiServerPort));
  drogon::app().setThreadNum(drogon_thread_num);
  LOG_INFO << "Number of thread is:" << drogon::app().getThreadNum();

  drogon::app().run();
  // return 0;
}

void ForkProcess() {
#if defined(_WIN32) || defined(_WIN64)
  // Windows-specific code to create a new process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  auto exe = commands::GetCortexBinary();
  std::string cmds =
      cortex_utils::GetCurrentPath() + "/" + exe + " --start-server";
  // Create child process
  if (!CreateProcess(
          NULL,  // No module name (use command line)
          const_cast<char*>(
              cmds.c_str()),  // Command line (replace with your actual executable)
          NULL,               // Process handle not inheritable
          NULL,               // Thread handle not inheritable
          FALSE,              // Set handle inheritance to FALSE
          0,                  // No creation flags
          NULL,               // Use parent's environment block
          NULL,               // Use parent's starting directory
          &si,                // Pointer to STARTUPINFO structure
          &pi))               // Pointer to PROCESS_INFORMATION structure
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
    // No need to configure LD_LIBRARY_PATH for macOS
#if !defined(__APPLE__) || !defined(__MACH__)
    const char* name = "LD_LIBRARY_PATH";
    auto data = getenv(name);
    std::string v;
    if (auto g = getenv(name); g) {
      v += g;
    }
    CTL_INF("LD_LIBRARY_PATH: " << v);
    auto data_path = file_manager_utils::GetCortexDataPath();
    auto llamacpp_path = data_path / "engines" / "cortex.llamacpp/";
    auto trt_path = data_path / "engines" / "cortex.tensorrt-llm/";
    auto new_v = trt_path.string() + ":" + llamacpp_path.string() + ":" + v;
    setenv(name, new_v.c_str(), true);
    CTL_INF("LD_LIBRARY_PATH: " << getenv(name));
#endif
    auto exe = commands::GetCortexBinary();
    std::string p = cortex_utils::GetCurrentPath() + "/" + exe;
    execl(p.c_str(), exe.c_str(), "--start-server", (char*)0);
  } else {
    // Parent process
    std::cout << "Server started" << std::endl;
  }
#endif
}

int main(int argc, char* argv[]) {
  // Stop the program if the system is not supported
  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info.arch == system_info_utils::kUnsupported ||
      system_info.os == system_info_utils::kUnsupported) {
    CTL_ERR("Unsupported OS or architecture: " << system_info.os << ", "
                                               << system_info.arch);
    return 1;
  }

  { file_manager_utils::CreateConfigFileIfNotExist(); }

  // Delete temporary file if it exists
  auto temp =
      file_manager_utils::GetExecutableFolderContainerPath() / "cortex_temp";
  if (std::filesystem::exists(temp)) {
    try {
      std::filesystem::remove(temp);
    } catch (const std::exception& e) {
      std::cerr << e.what() << '\n';
    }
  }

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
      RunServer();
      return 0;
    } else {
      bool verbose = false;
      for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0) {
          verbose = true;
        }
      }

      trantor::FileLogger asyncFileLogger;
      if (!verbose) {
        auto config = file_manager_utils::GetCortexConfig();
        std::filesystem::create_directories(
            std::filesystem::path(config.logFolderPath) /
            std::filesystem::path(cortex_utils::logs_folder));
        asyncFileLogger.setFileName(config.logFolderPath + "/" +
                                    cortex_utils::logs_cli_base_name);
        asyncFileLogger.setMaxLines(
            config.maxLogLines);  // Keep last 100000 lines
        asyncFileLogger.startLogging();
        trantor::Logger::setOutputFunction(
            [&](const char* msg, const uint64_t len) {
              asyncFileLogger.output_(msg, len);
            },
            [&]() { asyncFileLogger.flush(); });
      }
      CommandLineParser clp;
      clp.SetupCommand(argc, argv);
      return 0;
    }
  }

  ForkProcess();
  return 0;
}
