#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>
#include "utils/result.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <codecvt>
#include <locale>
#endif

namespace cortex {
class DylibPathManager {
  // for linux
  constexpr static auto kLdLibraryPath{"LD_LIBRARY_PATH"};

  struct DylibPath {
    std::filesystem::path path;
#if defined(_WIN32) || defined(_WIN64)
    DLL_DIRECTORY_COOKIE cookie;
#endif
  };

 public:
  cpp::result<void, std::string> RegisterPath(
      const std::string& key, std::vector<std::filesystem::path> paths);

  cpp::result<void, std::string> Unregister(const std::string& key);

 private:
  std::unordered_map<std::string, std::vector<DylibPath>> dylib_map_;
};
}  // namespace cortex
