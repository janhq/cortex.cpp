#include <iostream>

#ifdef _WIN32
#include <tlhelp32.h>
#include <windows.h>
#include <process.h>
using pid_t = DWORD;
#elif defined(__APPLE__) || defined(__linux__)
#include <errno.h>
#include <signal.h>
#endif
namespace process_status_utils {

inline bool IsProcessRunning(pid_t pid) {
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
}  // namespace process_status_utils