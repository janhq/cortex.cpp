#include "utils/process/utils.h"
#include <filesystem>
#include <sstream>
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

cpp::result<ProcessInfo, std::string> SpawnProcess(
    const std::vector<std::string>& command, const std::string& stdout_file,
    const std::string& stderr_file) {
  std::stringstream ss;
  for (const auto item : command) {
    ss << item << " ";
  }
  CTL_INF("Spawning process with command: " << ss.str());

  try {
#if defined(_WIN32)
    // Windows process creation
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    HANDLE hJob = NULL, hStdOut = NULL, hStdErr = NULL;

    // redirect stdout and stderr
    if (!stdout_file.empty() || !stderr_file.empty()) {
      si.dwFlags |= STARTF_USESTDHANDLES;

      // when STARTF_USESTDHANDLES is set, we have to explicitly inherit
      // parent's handles, otherwise subprocess may successfuly spawn but
      // exit immediately.
      si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
      si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
      si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

      SECURITY_ATTRIBUTES sa;
      sa.nLength = sizeof(sa);
      sa.lpSecurityDescriptor = NULL;
      sa.bInheritHandle = TRUE;

      if (!stdout_file.empty()) {
        hStdOut = CreateFileA(stdout_file.c_str(), FILE_APPEND_DATA,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hStdOut == INVALID_HANDLE_VALUE)
          throw std::runtime_error("Unable to create " + stdout_file +
                                   " to redirect stdout");

        si.hStdOutput = hStdOut;
      }
      if (!stderr_file.empty()) {
        hStdErr = CreateFileA(stderr_file.c_str(), FILE_APPEND_DATA,
                              FILE_SHARE_WRITE | FILE_SHARE_READ, &sa,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hStdErr == INVALID_HANDLE_VALUE) {
          if (hStdOut != NULL)
            CloseHandle(hStdOut);

          throw std::runtime_error("Unable to create " + stderr_file +
                                   " to redirect stderr");
        }

        si.hStdError = hStdErr;
      }
    }

    // Construct command line
    std::string cmd_line = ConstructWindowsCommandLine(command);

    // Convert string to char* for Windows API
    char command_buffer[4096];
    strncpy_s(command_buffer, cmd_line.c_str(), sizeof(command_buffer));

    // create a suspended process. we will resume it later after adding it to
    // a job (see below)
    if (!CreateProcessA(NULL,              // lpApplicationName
                        command_buffer,    // lpCommandLine
                        NULL,              // lpProcessAttributes
                        NULL,              // lpThreadAttributes
                        TRUE,              // bInheritHandles
                        CREATE_SUSPENDED,  // dwCreationFlags
                        NULL,              // lpEnvironment
                        NULL,              // lpCurrentDirectory
                        &si,               // lpStartupInfo
                        &pi                // lpProcessInformation
                        )) {
      if (hStdOut != NULL)
        CloseHandle(hStdOut);
      if (hStdErr != NULL)
        CloseHandle(hStdErr);
      throw std::runtime_error("Failed to create process on Windows");
    }

    // https://devblogs.microsoft.com/oldnewthing/20131209-00/?p=2433
    // resume thread after job object assignment to make sure child processes
    // will be spawned in the same job object.
    hJob = CreateJobObjectA(NULL, NULL);
    std::string err_msg;
    bool success = false;
    if (!AssignProcessToJobObject(hJob, pi.hProcess)) {
      err_msg = "Unable to assign process to job object";
    } else if (ResumeThread(pi.hThread) == (DWORD)(-1)) {
      err_msg = "Unable to resume thread";
    } else {
      success = true;
    }

    // clean up if not successful
    if (!success) {
      TerminateProcess(pi.hProcess, 0);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      CloseHandle(hJob);
      if (hStdOut != NULL)
        CloseHandle(hStdOut);
      if (hStdErr != NULL)
        CloseHandle(hStdErr);
      throw std::runtime_error(err_msg);
    }

    // Close handles to avoid resource leaks
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    ProcessInfo proc_info;
    proc_info.pid = pi.dwProcessId;
    proc_info.hJob = hJob;
    proc_info.hStdOut = hStdOut;
    proc_info.hStdErr = hStdErr;

    return proc_info;

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
          int rc = posix_spawn_file_actions_addopen(&action, STDOUT_FILENO,
                                                    stdout_file.data(),
                                                    O_WRONLY | O_APPEND, 0);
          if (rc != 0) {
            posix_spawn_file_actions_destroy(action_ptr);
            throw std::runtime_error("Unable to add stdout to file action");
          }
        }
      }

      if (!stderr_file.empty()) {
        if (std::filesystem::exists(stderr_file)) {
          int rc = posix_spawn_file_actions_addopen(&action, STDERR_FILENO,
                                                    stderr_file.data(),
                                                    O_WRONLY | O_APPEND, 0);
          if (rc != 0) {
            posix_spawn_file_actions_destroy(action_ptr);
            throw std::runtime_error("Unable to add stderr to file action");
          }
        }
      }
    }

    // Use posix_spawn for cross-platform compatibility
    auto spawn_result = posix_spawn(&pid,                // pid output
                                    command[0].c_str(),  // executable path
                                    action_ptr,          // file actions
                                    NULL,                // spawn attributes
                                    argv.data(),         // argument vector
                                    environ  // environment (inherit)
    );

    // NOTE: it seems like it's ok to destroy this immediately before
    // subprocess terminates.
    if (action_ptr != NULL) {
      posix_spawn_file_actions_destroy(action_ptr);
    }

    if (spawn_result != 0) {
      throw std::runtime_error("Failed to spawn process");
    }

    ProcessInfo proc_info;
    proc_info.pid = pid;

    return proc_info;

#else
#error Unsupported platform
#endif
  } catch (const std::exception& e) {
    LOG_ERROR << "Process spawning error: " << e.what();
    return cpp::fail(e.what());
  }
}

static void SetProcessTerminated(ProcessInfo& proc_info) {
  if (proc_info.pid == PID_TERMINATED)
    return;

  proc_info.pid = PID_TERMINATED;

  // close handles on Windows
#if defined(_WIN32)
  CloseHandle(proc_info.hJob);
  proc_info.hJob = NULL;
  if (proc_info.hStdOut != NULL) {
    CloseHandle(proc_info.hStdOut);
    proc_info.hStdOut = NULL;
  }
  if (proc_info.hStdErr != NULL) {
    CloseHandle(proc_info.hStdErr);
    proc_info.hStdErr = NULL;
  }
#endif
}

bool IsProcessAlive(ProcessInfo& proc_info) {
  if (proc_info.pid == PID_TERMINATED)
    return false;

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
      if (processEntry.th32ProcessID == proc_info.pid) {
        CloseHandle(snapshot);
        return true;
      }
    } while (Process32Next(snapshot, &processEntry));
  }

  // pid not found in snapshot -> process has terminated.
  CloseHandle(snapshot);
  SetProcessTerminated(proc_info);
  return false;

#elif defined(__APPLE__) || defined(__linux__)
  // Unix-like systems (Linux and macOS) implementation

  // NOTE: kill(pid, 0) only works if the process has been reaped.
  // if the process has terminated but not reaped (exit status is still
  // stored in the process table), kill(pid, 0) still returns 0.

  // Try to send signal 0 to the process
  // This doesn't actually send a signal but checks if we can send signals to the process
  // Process exists and we have permission to send it signals
  // if (kill(proc_info.pid, 0) == 0) {
  //   return true;
  // }

  // // process exists but we don't have permission to send signal
  // if (errno == EPERM)
  //   return true;

  if (waitpid(proc_info.pid, NULL, WNOHANG) == 0)
    return true;
  SetProcessTerminated(proc_info);
  return false;
#else
#error "Unsupported platform"
#endif
}

bool WaitProcess(ProcessInfo& proc_info) {
  if (proc_info.pid == PID_TERMINATED)
    return true;

  bool success;

#if defined(_WIN32)
  // NOTE: OpenProcess() may fail if the process has terminated.
  HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, proc_info.pid);
  success = WaitForSingleObject(hProcess, INFINITE) == WAIT_OBJECT_0;
  CloseHandle(hProcess);
#elif defined(__APPLE__) || defined(__linux__)
  // NOTE: waitpid() may fail if the process has terminated and the OS
  // has reaped it (i.e. clear its exit status).
  success = waitpid(proc_info.pid, NULL, 0) == proc_info.pid;
#else
#error "Unsupported platform"
#endif

  if (success)
    SetProcessTerminated(proc_info);
  return success;
}

bool KillProcess(ProcessInfo& proc_info) {
  if (proc_info.pid == PID_TERMINATED)
    return true;

  bool success;

#if defined(_WIN32)
  success = TerminateJobObject(proc_info.hJob, 0) == 0;
#elif defined(__APPLE__) || defined(__linux__)
  // we send SIGTERM to subprocess. we trust that this subprocess will
  // propagate SIGTERM correctly to its children processes.
  success = kill(proc_info.pid, SIGTERM) == 0;
#else
#error "Unsupported platform"
#endif

  if (success)
    SetProcessTerminated(proc_info);
  return success;
}

}  // namespace cortex::process
