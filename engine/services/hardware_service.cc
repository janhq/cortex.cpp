// clang-format off
#include "cli/commands/server_start_cmd.h"
// clang-format on
#include "hardware_service.h"
#if defined(_WIN32) || defined(_WIN64)
#include <minwindef.h>
#include <processenv.h>
#endif
#include "cli/commands/cortex_upd_cmd.h"
#include "utils/cortex_utils.h"

namespace services {

namespace {
bool TryConnectToServer(const std::string& host, int port) {
  constexpr const auto kMaxRetry = 3u;
  auto count = 0u;
  // Check if server is started
  while (true) {
    if (commands::IsServerAlive(host, port))
      break;
    // Wait for server up
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (count++ == kMaxRetry) {
      std::cerr << "Could not start server" << std::endl;
      return false;
    }
  }
  return true;
}
}  // namespace

HardwareInfo HardwareService::GetHardwareInfo() {
  return HardwareInfo{.cpu = hardware::GetCPUInfo(),
                      .os = hardware::GetOSInfo(),
                      .ram = hardware::GetMemoryInfo(),
                      .storage = hardware::GetStorageInfo(),
                      .gpus = hardware::GetGPUInfo(),
                      .power = hardware::GetPowerInfo()};
}

bool HardwareService::Restart(const std::string& host, int port,
                              const ActivateHardwareConfig& ahc) {
  auto exe = commands::GetCortexServerBinary();
  auto get_config_file_path = []() -> std::string {
    if (file_manager_utils::cortex_config_file_path.empty()) {
      return file_manager_utils::GetConfigurationPath().string();
    }
    return file_manager_utils::cortex_config_file_path;
  };

  auto get_data_folder_path = []() -> std::string {
    if (file_manager_utils::cortex_data_folder_path.empty()) {
      return file_manager_utils::GetCortexDataPath().string();
    }
    return file_manager_utils::cortex_data_folder_path;
  };

  auto set_env = [](const std::string& name, const std::string& value,
                    bool is_override = true) -> bool {
#if defined(_WIN32) || defined(_WIN64)
    return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
    return setenv(name.c_str(), value.c_str(), is_override) == 0;
#endif
  };

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
  std::string cuda_visible_devices = "";
  for (auto i : ahc.gpus) {
    if (!cuda_visible_devices.empty())
      cuda_visible_devices += ",";
    cuda_visible_devices += std::to_string(i);
  }
  if (cuda_visible_devices.empty())
    cuda_visible_devices += " ";
  // Set the CUDA_VISIBLE_DEVICES environment variable
  if (!set_env("CUDA_VISIBLE_DEVICES", cuda_visible_devices)) {
    LOG_WARN << "Error setting CUDA_VISIBLE_DEVICES";
    return false;
  }

  const char* value = std::getenv("CUDA_VISIBLE_DEVICES");
  if (value) {
    LOG_INFO << "CUDA_VISIBLE_DEVICES is set to: " << value;
  } else {
    LOG_WARN << "CUDA_VISIBLE_DEVICES is not set.";
  }
#endif

#if defined(_WIN32) || defined(_WIN64)
  // Windows-specific code to create a new process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  std::string params = "--start-server";
  params += " --config_file_path " + get_config_file_path();
  params += " --data_folder_path " + get_data_folder_path();
  std::string cmds = cortex_utils::GetCurrentPath() + "/" + exe + " " + params;
  // Create child process
  if (!CreateProcess(
          NULL,  // No module name (use command line)
          const_cast<char*>(
              cmds.c_str()),  // Command line (replace with your actual executable)
          NULL,               // Process handle not inheritable
          NULL,               // Thread handle not inheritable
          TRUE,               // Handle inheritance
          0,                  // No creation flags
          NULL,               // Use parent's environment block
          NULL,               // Use parent's starting directory
          &si,                // Pointer to STARTUPINFO structure
          &pi))               // Pointer to PROCESS_INFORMATION structure
  {
    std::cout << "Could not start server: " << GetLastError() << std::endl;
    return false;
  } else {
    if (!TryConnectToServer(host, port)) {
      return false;
    }
    std::cout << "Server started" << std::endl;
    std::cout << "API Documentation available at: http://" << host << ":"
              << port << std::endl;
  }

#else
  // Unix-like system-specific code to fork a child process
  pid_t pid = fork();

  if (pid < 0) {
    // Fork failed
    std::cerr << "Could not start server: " << std::endl;
    return false;
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
    auto data_path = file_manager_utils::GetEnginesContainerPath();
    auto llamacpp_path = data_path / "cortex.llamacpp/";
    auto trt_path = data_path / "cortex.tensorrt-llm/";
    if (!std::filesystem::exists(llamacpp_path)) {
      std::filesystem::create_directory(llamacpp_path);
    }

    auto new_v = trt_path.string() + ":" + llamacpp_path.string() + ":" + v;
    setenv(name, new_v.c_str(), true);
    CTL_INF("LD_LIBRARY_PATH: " << getenv(name));
#endif
    std::string p = cortex_utils::GetCurrentPath() + "/" + exe;
    execl(p.c_str(), exe.c_str(), "--start-server", "--config_file_path",
          get_config_file_path().c_str(), "--data_folder_path",
          get_data_folder_path().c_str(), (char*)0);
  } else {
    // Parent process
    if (!TryConnectToServer(host, port)) {
      return false;
    }
    std::cout << "Server started" << std::endl;
    std::cout << "API Documentation available at: http://" << host << ":"
              << port << std::endl;
  }
#endif
  return true;
}
}  // namespace services