#include "utils/process/utils.h"
#include <filesystem>
#include "utils/logging_utils.h"

#if defined(_WIN32)
#include <tlhelp32.h>
#elif defined(__APPLE__) || defined(__linux__)
extern char** environ;  // environment variables
#include <errno.h>
#include <fcntl.h>
#endif

namespace cortex::process {

std::string ConstructWindowsCommandLine(const std::vector<std::string>& args) {
  std::string cmd_line;
  for (const auto& arg : args) {
    // Simple escaping for Windows command line
    std::string escaped_arg = arg;
    if (escaped_arg.find(' ') != std::string::npos) {
      // Wrap in quotes and escape existing quotes
      for (char& c : escaped_arg) {
        if (c == '"')
          c = '\\';
      }
      escaped_arg = "\"" + escaped_arg + "\"";
    }
    cmd_line += escaped_arg + " ";
  }
  return cmd_line;
}

std::vector<char*> ConvertToArgv(const std::vector<std::string>& args) {
  std::vector<char*> argv;
  for (const auto& arg : args) {
    argv.push_back(const_cast<char*>(arg.c_str()));
  }
  argv.push_back(nullptr);
  return argv;
}

pid_t SpawnProcess(const std::vector<std::string>& command,
                   const std::string stdout_file,
                   const std::string stderr_file,
                   bool wait) {
  try {
#if defined(_WIN32)
    // Windows process creation
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    // Construct command line
    std::string cmd_line = ConstructWindowsCommandLine(command);

    // Convert string to char* for Windows API
    char command_buffer[4096];
    strncpy_s(command_buffer, cmd_line.c_str(), sizeof(command_buffer));

    if (!CreateProcessA(NULL,            // lpApplicationName
                        command_buffer,  // lpCommandLine
                        NULL,            // lpProcessAttributes
                        NULL,            // lpThreadAttributes
                        FALSE,           // bInheritHandles
                        0,               // dwCreationFlags
                        NULL,            // lpEnvironment
                        NULL,            // lpCurrentDirectory
                        &si,             // lpStartupInfo
                        &pi              // lpProcessInformation
                        )) {
      throw std::runtime_error("Failed to create process on Windows");
    }

    // Store the process ID
    pid_t pid = pi.dwProcessId;

    // wait for process to terminate
    if (wait)
      WaitForSingleObject(pi.hProcess, INFINITE);

    // Close handles to avoid resource leaks
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return pid;

#elif defined(__APPLE__) || defined(__linux__)
    // POSIX process creation
    pid_t pid;

    // Convert command vector to char*[]
    auto argv = ConvertToArgv(command);

    // redirect stdout and stderr
    // caller should make sure the redirect files exist.
    posix_spawn_file_actions_t* action_ptr = NULL;

    if (!stdout_file.empty() || !stderr_file.empty()) {
      posix_spawn_file_actions_t action;
      posix_spawn_file_actions_init(&action);
      action_ptr = &action;

      if (!stdout_file.empty()) {
        if (std::filesystem::exists(stdout_file)) {
          posix_spawn_file_actions_addopen(&action, STDOUT_FILENO,
                                           stdout_file.data(),
                                           O_WRONLY | O_APPEND, 0);
        }
      }

      if (!stderr_file.empty()) {
        if (std::filesystem::exists(stderr_file)) {
          posix_spawn_file_actions_addopen(&action, STDERR_FILENO,
                                           stderr_file.data(),
                                           O_WRONLY | O_APPEND, 0);
        }
      }
    }

    // Use posix_spawn for cross-platform compatibility
    auto spawn_result = posix_spawnp(&pid,                // pid output
                                     command[0].c_str(),  // executable path
                                     action_ptr,          // file actions
                                     NULL,                // spawn attributes
                                     argv.data(),         // argument vector
                                     environ  // environment (inherit)
    );

    // NOTE: it seems like it's ok to destroy this immediately before
    // subprocess terminates.
    posix_spawn_file_actions_destroy(action_ptr);

    if (spawn_result != 0) {
      throw std::runtime_error("Failed to spawn process");
    }

    // wait for process to terminate
    if (wait)
      waitpid(pid, NULL, 0);

    return pid;

#else
#error Unsupported platform
#endif
  } catch (const std::exception& e) {
    LOG_ERROR << "Process spawning error: " << e.what();
    return -1;
  }
}

bool IsProcessAlive(pid_t pid) {
#ifdef _WIN32
  // Windows implementation
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    return false;
  }

  PROCESSENTRY32 processEntry = {0};
  processEntry.dwSize = sizeof(processEntry);

  if (Process32First(snapshot, &processEntry)) {
    do {
      if (processEntry.th32ProcessID == pid) {
        CloseHandle(snapshot);
        return true;
      }
    } while (Process32Next(snapshot, &processEntry));
  }

  CloseHandle(snapshot);
  return false;

#elif defined(__APPLE__) || defined(__linux__)
  // Unix-like systems (Linux and macOS) implementation
  if (pid <= 0) {
    return false;
  }

  // Try to send signal 0 to the process
  // This doesn't actually send a signal but checks if we can send signals to the process
  int result = kill(pid, 0);

  if (result == 0) {
    return true;  // Process exists and we have permission to send it signals
  }

  return errno != ESRCH;  // ESRCH means "no such process"
#else
#error "Unsupported platform"
#endif
}

bool KillProcess(pid_t pid) {
#if defined(_WIN32)
  HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
  if (hProcess == NULL) {
    LOG_ERROR << "Failed to open process";
    return false;
  }

  bool is_success = TerminateProcess(hProcess, 0) == TRUE;
  CloseHandle(hProcess);
  return is_success;
#elif defined(__APPLE__) || defined(__linux__)
  // NOTE: should we use SIGKILL here to be consistent with Windows?
  return kill(pid, SIGTERM) == 0;
#else
#error "Unsupported platform"
#endif
}

}  // namespace cortex::process
