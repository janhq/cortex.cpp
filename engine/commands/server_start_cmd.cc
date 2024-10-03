#include "server_start_cmd.h"
#include "commands/cortex_upd_cmd.h"
#include "httplib.h"
#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {

namespace {
bool TryConnectToServer(const std::string& host, int port) {
  constexpr const auto kMaxRetry = 3u;
  auto count = 0u;
  // Check if server is started
  while (true) {
    if (IsServerAlive(host, port))
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

ServerStartCmd::ServerStartCmd() {}

bool ServerStartCmd::Exec(const std::string& host, int port) {
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
    return false;
  } else {
    if(!TryConnectToServer(host, port)) {
        return false;
    }
    std::cout << "Server started" << std::endl;
    std::cout << "API Documentation available at: http://" << host << ":" << port << std::endl;
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
    if(!TryConnectToServer(host, port)) {
        return false;
    }
    std::cout << "Server started" << std::endl;
    std::cout << "API Documentation available at: http://" << host << ":" << port << std::endl;
  }
#endif
  return true;
}

};  // namespace commands