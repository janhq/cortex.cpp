#include "utils/process/utils.h"
#include "utils/logging_utils.h"

#if defined(_WIN32)
#include <tlhelp32.h>
#elif defined(__APPLE__) || defined(__linux__)
extern char** environ;  // environment variables
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
    const std::vector<std::string>& command) {
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

    // create a suspended process. we will resume it later after adding it to
    // a job (see below)
    if (!CreateProcessA(NULL,              // lpApplicationName
                        command_buffer,    // lpCommandLine
                        NULL,              // lpProcessAttributes
                        NULL,              // lpThreadAttributes
                        FALSE,             // bInheritHandles
                        CREATE_SUSPENDED,  // dwCreationFlags
                        NULL,              // lpEnvironment
                        NULL,              // lpCurrentDirectory
                        &si,               // lpStartupInfo
                        &pi                // lpProcessInformation
                        )) {
      throw std::runtime_error("Failed to create process on Windows");
    }

    // https://devblogs.microsoft.com/oldnewthing/20131209-00/?p=2433
    // resume thread after job object assignment to make sure child processes
    // will be spawned in the same job object.
    HANDLE hJob = CreateJobObjectA(NULL, NULL);
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
      throw std::runtime_error(err_msg);
    }

    // Close handles to avoid resource leaks
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    ProcessInfo proc_info;
    proc_info.pid = pi.dwProcessId;
    proc_info.hJob = hJob;

    return proc_info;

#elif defined(__APPLE__) || defined(__linux__)
    // POSIX process creation
    pid_t pid;

    // Convert command vector to char*[]
    auto argv = ConvertToArgv(command);

    // Use posix_spawn for cross-platform compatibility
    auto spawn_result = posix_spawn(&pid,                // pid output
                                    command[0].c_str(),  // executable path
                                    NULL,                // file actions
                                    NULL,                // spawn attributes
                                    argv.data(),         // argument vector
                                    environ  // environment (inherit)
    );

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

bool IsProcessAlive(const ProcessInfo& proc_info) {
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

  CloseHandle(snapshot);
  return false;

#elif defined(__APPLE__) || defined(__linux__)
  // Unix-like systems (Linux and macOS) implementation

  // Try to send signal 0 to the process
  // This doesn't actually send a signal but checks if we can send signals to the process
  int result = kill(proc_info.pid, 0);

  if (result == 0) {
    return true;  // Process exists and we have permission to send it signals
  }

  return errno != ESRCH;  // ESRCH means "no such process"
#else
#error "Unsupported platform"
#endif
}

bool KillProcess(ProcessInfo& proc_info) {
#if defined(_WIN32)
  BOOL rc = TerminateJobObject(proc_info.hJob, 0);
  if (rc == 0) {
    CloseHandle(proc_info.hJob);
    proc_info.hJob = NULL;
    return true;
  }
  return false;
  // HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, proc_info.pid);
  // if (hProcess == NULL) {
  //   LOG_ERROR << "Failed to open process";
  //   return false;
  // }

  // bool is_success = TerminateProcess(hProcess, 0) == TRUE;
  // CloseHandle(hProcess);
  // return is_success;
#elif defined(__APPLE__) || defined(__linux__)
  return kill(proc_info.pid, SIGTERM) == 0;
#else
#error "Unsupported platform"
#endif
}

}  // namespace cortex::process
