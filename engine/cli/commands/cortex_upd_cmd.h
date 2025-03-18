#pragma once

#include <string>
#include <vector>
#include "services/download_service.h"
#if !defined(_WIN32)
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "utils/file_manager_utils.h"

namespace commands {
#ifndef CORTEX_VARIANT
#define CORTEX_VARIANT file_manager_utils::kProdVariant
#endif
constexpr const auto kNightlyHost = "delta.jan.ai";
const std::string kCortexBinary = "cortex";
const std::string kCortexServerBinary = "cortex-server";
constexpr const auto kBetaComp = "-rc";
constexpr const auto kReleaseFormat = "network-installer";
constexpr const auto kTimeoutCheckUpdate = std::chrono::milliseconds(1000);

inline std::string GetRole() {
#if defined(_WIN32)
  return "";
#else
  // not root
  if (getuid()) {
    return "sudo ";
  } else {
    return "";
  }
#endif
}

inline std::string GetCortexBinary() {
#if defined(_WIN32)
  constexpr const bool has_exe = true;
#else
  constexpr const bool has_exe = false;
#endif
  if (CORTEX_VARIANT == file_manager_utils::kNightlyVariant) {
    return has_exe ? kCortexBinary + "-nightly.exe"
                   : kCortexBinary + "-nightly";
  } else if (CORTEX_VARIANT == file_manager_utils::kBetaVariant) {
    return has_exe ? kCortexBinary + "-beta.exe" : kCortexBinary + "-beta";
  } else {
    return has_exe ? kCortexBinary + ".exe" : kCortexBinary;
  }
}

inline std::string GetCortexServerBinary() {
#if defined(_WIN32)
  constexpr const bool has_exe = true;
#else
  constexpr const bool has_exe = false;
#endif
  if (CORTEX_VARIANT == file_manager_utils::kNightlyVariant) {
    return has_exe ? kCortexServerBinary + "-nightly.exe"
                   : kCortexServerBinary + "-nightly";
  } else if (CORTEX_VARIANT == file_manager_utils::kBetaVariant) {
    return has_exe ? kCortexServerBinary + "-beta.exe"
                   : kCortexServerBinary + "-beta";
  } else {
    return has_exe ? kCortexServerBinary + ".exe" : kCortexServerBinary;
  }
}

inline std::string GetHostName() {
  if (CORTEX_VARIANT == file_manager_utils::kNightlyVariant) {
    return "delta.jan.ai";
  } else {
    return "api.github.com";
  }
}

inline std::vector<std::string> GetReleasePath() {
  if (CORTEX_VARIANT == file_manager_utils::kNightlyVariant) {
    return {"cortex", "latest", "version.json"};
  } else if (CORTEX_VARIANT == file_manager_utils::kBetaVariant) {
    return {"repos", "menloresearch", "cortex.cpp", "releases"};
  } else {
    return {"repos", "menloresearch", "cortex.cpp", "releases", "latest"};
  }
}

std::optional<std::string> CheckNewUpdate(
    std::optional<std::chrono::milliseconds> timeout);

// This class manages the 'cortex update' command functionality
// There are three release types available:
// - Stable: Only retrieves the latest version
// - Beta: Allows retrieval of the latest beta version and specific versions using the -v flag
// - Nightly: Enables retrieval of the latest nightly build and specific versions using the -v flag
class CortexUpdCmd {
 public:
  explicit CortexUpdCmd(std::shared_ptr<DownloadService> download_service)
      : download_service_{download_service} {};

  void Exec(const std::string& v, bool force = false);

 private:
  std::shared_ptr<DownloadService> download_service_;

  bool GetStable(const std::string& v);
  bool GetBeta(const std::string& v);
  std::optional<std::string> HandleGithubRelease(const Json::Value& assets,
                                                 const std::string& os_arch);
  bool GetNightly(const std::string& v);

  // For Linux, we use different approach to update
  // The installation bash script will perform the following tasks (all logic for update will be put into the bash script):
  // - Detect whether the user is performing a new installation or an update.
  // - Detect whether a .deb package needs to be installed or if the binary file should be installed directly.
  bool GetLinuxInstallScript(const std::string& v, const std::string& channel);
};
}  // namespace commands
