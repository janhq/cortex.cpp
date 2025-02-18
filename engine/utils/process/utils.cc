#include "utils/process/utils.h"
#include "utils/logging_utils.h"
#include <filesystem>

#if defined(__APPLE__) || defined(__linux__)
extern char **environ;  // environment variables
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
                   const std::optional<std::string> stdout_file,
                   const std::optional<std::string> stderr_file) {
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
    posix_spawn_file_actions_t *action_ptr = NULL;

    if (stdout_file.has_value() || stderr_file.has_value()) {
      posix_spawn_file_actions_t action;
      posix_spawn_file_actions_init(&action);
      action_ptr = &action;

      if (stdout_file.has_value()) {
        std::string stdout_file_val = stdout_file.value();
        if (std::filesystem::exists(stdout_file_val)) {
          posix_spawn_file_actions_addopen(&action, STDOUT_FILENO,
                                           stdout_file_val.data(),
                                           O_WRONLY | O_APPEND, 0);
        }
      }

      if (stderr_file.has_value()) {
        std::string stderr_file_val = stderr_file.value();
        if (std::filesystem::exists(stderr_file_val)) {
          posix_spawn_file_actions_addopen(&action, STDERR_FILENO,
                                           stderr_file_val.data(),
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

    // NOTE: only destroy this when process ends?
    // posix_spawn_file_actions_destroy(action_pr);

    if (spawn_result != 0) {
      throw std::runtime_error("Failed to spawn process");
    }

    return pid;

#else
#error Unsupported platform
#endif
  } catch (const std::exception& e) {
    LOG_ERROR << "Process spawning error: " << e.what();
    return -1;
  }
}

}  // namespace cortex::process