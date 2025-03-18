#include <memory>
#include "command_line_parser.h"
#include "commands/cortex_upd_cmd.h"
#include "openssl/ssl.h"
#include "services/download_service.h"
#include "utils/archive_utils.h"
#include "utils/cortex_utils.h"
#include "utils/file_logger.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/system_info_utils.h"
#include "utils/widechar_conv.h"

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

#include <codecvt>
#include <locale>

void RemoveBinaryTempFileIfExists() {
  auto temp =
      file_manager_utils::GetExecutableFolderContainerPath() / "cortex_temp";
  if (std::filesystem::exists(temp)) {
    try {
      std::filesystem::remove(temp);
    } catch (const std::exception& e) {
      std::cerr << e.what() << '\n';
    }
  }
}

void SetupLogger(trantor::FileLogger& async_logger, bool verbose) {
  if (!verbose) {
    auto config = file_manager_utils::GetCortexConfig();

    std::filesystem::create_directories(
#if defined(_WIN32)
        std::filesystem::path(cortex::wc::Utf8ToWstring(config.logFolderPath)) /
#else
        std::filesystem::path(config.logFolderPath) /
#endif
        std::filesystem::path(cortex_utils::logs_folder));

    // Do not need to use u8path here because trantor handles itself
    async_logger.setFileName(
        (std::filesystem::path(config.logFolderPath) /
         std::filesystem::path(cortex_utils::logs_cli_base_name))
            .string());
    async_logger.setMaxLines(config.maxLogLines);  // Keep last 100000 lines
    async_logger.startLogging();
    trantor::Logger::setOutputFunction(
        [&](const char* msg, const uint64_t len) {
          async_logger.output_(msg, len);
        },
        [&]() { async_logger.flush(); });
  }
}

void InstallServer() {
#if !defined(_WIN32)
  if (getuid()) {
    CLI_LOG("Error: Not root user. Please run with sudo.");
    return;
  }
#endif
  auto cuc = commands::CortexUpdCmd(std::make_shared<DownloadService>());
  cuc.Exec({}, true /*force*/);
}

int main(int argc, char* argv[]) {
  // Stop the program if the system is not supported
  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info->arch == system_info_utils::kUnsupported ||
      system_info->os == system_info_utils::kUnsupported) {
    CTL_ERR("Unsupported OS or architecture: " << system_info->os << ", "
                                               << system_info->arch);
    return 1;
  }

  SSL_library_init();
  curl_global_init(CURL_GLOBAL_DEFAULT);

  bool should_install_server = false;
  bool verbose = false;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--config_file_path") == 0) {
      file_manager_utils::cortex_config_file_path = argv[i + 1];

    } else if (strcmp(argv[i], "--data_folder_path") == 0) {
      file_manager_utils::cortex_data_folder_path = argv[i + 1];
    } else if ((strcmp(argv[i], "--server") == 0) &&
               (strcmp(argv[i - 1], "update") == 0)) {
      should_install_server = true;
    } else if (strcmp(argv[i], "--verbose") == 0) {
      verbose = true;
    }
  }

  {
    auto result = file_manager_utils::CreateConfigFileIfNotExist();
    if (result.has_error()) {
      CTL_ERR("Error creating config file: " << result.error());
    }
    namespace fmu = file_manager_utils;
    // Override data folder path if it is configured and changed
    if (!fmu::cortex_data_folder_path.empty()) {
      auto cfg = file_manager_utils::GetCortexConfig();
      if (cfg.dataFolderPath != fmu::cortex_data_folder_path ||
          cfg.logFolderPath != fmu::cortex_data_folder_path) {
        cfg.dataFolderPath = fmu::cortex_data_folder_path;
        cfg.logFolderPath = fmu::cortex_data_folder_path;
        auto config_path = file_manager_utils::GetConfigurationPath();
        auto result =
            config_yaml_utils::CortexConfigMgr::GetInstance().DumpYamlConfig(
                cfg, config_path.string());
        if (result.has_error()) {
          CTL_ERR("Error update " << config_path.string() << result.error());
        }
      }
    }
  }

  RemoveBinaryTempFileIfExists();

  auto should_check_for_latest_llamacpp_version = true;
  auto now = std::chrono::system_clock::now();

  // read the yaml to see the last time we check for update
  auto config = file_manager_utils::GetCortexConfig();
  if (config.checkedForLlamacppUpdateAt != 0) {
    // if it passed a day, then we should check
    auto last_check =
        std::chrono::system_clock::time_point(
            std::chrono::milliseconds(config.checkedForLlamacppUpdateAt)) +
        std::chrono::hours(24);
    should_check_for_latest_llamacpp_version = now > last_check;
  }

  if (false) {
    std::thread t1([]() {
      // TODO: namh current we only check for llamacpp. Need to add support for other engine
      auto get_latest_version = []() -> cpp::result<std::string, std::string> {
        try {
          auto res = github_release_utils::GetReleaseByVersion(
              "menloresearch", "cortex.llamacpp", "latest");
          if (res.has_error()) {
            CTL_ERR("Failed to get latest llama.cpp version: " << res.error());
            return cpp::fail("Failed to get latest llama.cpp version: " +
                             res.error());
          }
          CTL_INF("Latest llamacpp version: " << res->tag_name);
          return res->tag_name;
        } catch (const std::exception& e) {
          CTL_ERR("Failed to get latest llama.cpp version: " << e.what());
          return cpp::fail("Failed to get latest llama.cpp version: " +
                           std::string(e.what()));
        }
      };

      auto res = get_latest_version();
      if (res.has_error()) {
        CTL_ERR("Failed to get latest llama.cpp version: " << res.error());
        return;
      }

      auto now = std::chrono::system_clock::now();
      CTL_DBG("latest llama.cpp version: " << res.value());
      auto config = file_manager_utils::GetCortexConfig();
      config.checkedForLlamacppUpdateAt =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              now.time_since_epoch())
              .count();
      config.latestLlamacppRelease = res.value();

      auto upd_config_res =
          config_yaml_utils::CortexConfigMgr::GetInstance().DumpYamlConfig(
              config, file_manager_utils::GetConfigurationPath().string());
      if (upd_config_res.has_error()) {
        CTL_ERR("Failed to update config file: " << upd_config_res.error());
      } else {
        CTL_INF("Updated config file with latest llama.cpp version: "
                << res.value());
      }
    });
    t1.detach();
  }

  static trantor::FileLogger async_file_logger;
  SetupLogger(async_file_logger, verbose);

  if (should_install_server) {
    InstallServer();
    return 0;
  }

  // Check if server exists, if not notify to user to install server
  auto exe = commands::GetCortexServerBinary();
  auto server_binary_path =
      file_manager_utils::GetExecutableFolderContainerPath() / exe;
  if (!std::filesystem::exists(server_binary_path)) {
    std::cout << CORTEX_CPP_VERSION
              << " requires server binary, to install server, run: "
              << commands::GetRole() << commands::GetCortexBinary()
              << " update --server" << std::endl;
    return 0;
  }

  CommandLineParser clp;
  clp.SetupCommand(argc, argv);
  return 0;
}
