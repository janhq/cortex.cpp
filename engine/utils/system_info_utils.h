#pragma once

#include <ostream>

namespace system_info_utils {
struct SystemInfo {
  std::string os;
  std::string arch;
};

constexpr static auto kUnsupported{"Unsupported"};

inline SystemInfo GetSystemInfo() {
  std::ostringstream arch;
  std::ostringstream os;

#if defined(__i386__) || defined(__x86_64__)
  arch << "amd64";
#elif defined(__arm__) || defined(__arm64__) || defined(__aarch64__)
  arch << "arm64";
#else
  arch << kUnsupported;
#endif

#if defined(__APPLE__) && defined(__MACH__)
  os << "mac";
#elif defined(__linux__)
  os << "linux";
#elif defined(_WIN32)
  os << "windows";
#else
  os << kUnsupported;
#endif
  return SystemInfo{os.str(), arch.str()};
}
}  // namespace system_info_utils