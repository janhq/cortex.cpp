#pragma once
#include <string>
#if !defined(_WIN32)
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "utils/file_manager_utils.h"

namespace commands {

// This class manages the 'cortex update' command functionality
// There are three release types available:
// - Stable: Only retrieves the latest version
// - Beta: Allows retrieval of the latest beta version and specific versions using the -v flag
// - Nightly: Enables retrieval of the latest nightly build and specific versions using the -v flag
class CortexUpdServerCmd {
 public:
  explicit CortexUpdServerCmd(std::shared_ptr<DownloadService> download_service)
      : download_service_{download_service} {};

  void Exec(const std::string& v);

 private:
  std::shared_ptr<DownloadService> download_service_;

  bool GetStable(const std::string& v);
  bool GetBeta(const std::string& v);
  bool HandleGithubRelease(const nlohmann::json& assets,
                           const std::string& os_arch);
  bool GetNightly(const std::string& v);
};
}  // namespace commands
