#include <memory>
#include "controllers/command_line_parser.h"
#include "cortex-common/cortexpythoni.h"
#include "services/model_service.h"
#include "utils/archive_utils.h"
#include "utils/cortex_utils.h"
#include "utils/dylib.h"
#include "utils/event_processor.h"
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
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>  // for readlink()
#elif defined(_WIN32)
#include <windows.h>
#undef max
#else
#error "Unsupported platform!"
#endif

int main(int argc, char* argv[]) {
  // Stop the program if the system is not supported
  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info->arch == system_info_utils::kUnsupported ||
      system_info->os == system_info_utils::kUnsupported) {
    CTL_ERR("Unsupported OS or architecture: " << system_info->os << ", "
                                               << system_info->arch);
    return 1;
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--config_file_path") == 0) {
      file_manager_utils::cortex_config_file_path = argv[i + 1];

    } else if(strcmp(argv[i], "--data_folder_path") == 0) {
      file_manager_utils::cortex_data_folder_path = argv[i + 1];
    }
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
        std::string abs_path =
            cortex_utils::GetCurrentPath() + kPythonRuntimeLibPath;
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

  if (argc > 1 && strcmp(argv[1], "--start-server") == 0) {
    // RunServer();
    return 0;
  }

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
    asyncFileLogger.setMaxLines(config.maxLogLines);  // Keep last 100000 lines
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
