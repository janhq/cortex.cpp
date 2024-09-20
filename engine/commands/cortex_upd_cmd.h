#pragma once
#include <string>
#if !defined(_WIN32)
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "httplib.h"
#include "nlohmann/json.hpp"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {
#ifndef CORTEX_VARIANT
#define CORTEX_VARIANT file_manager_utils::kProdVariant
#endif
constexpr const auto kNightlyHost = "delta.jan.ai";
constexpr const auto kNightlyFileName = "cortex-nightly.tar.gz";
const std::string kCortexBinary = "cortex";
constexpr const auto kBetaComp = "-rc";
constexpr const auto kReleaseFormat = ".tar.gz";
constexpr const auto kTimeoutCheckUpdate = std::chrono::milliseconds(1000);

inline std::string GetRole() {
#if defined(_WIN32)
  return "";
#else
  return "sudo ";
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

inline std::string GetHostName() {
  if (CORTEX_VARIANT == file_manager_utils::kNightlyVariant) {
    return "https://delta.jan.ai";
  } else {
    return "https://api.github.com";
  }
}

inline std::string GetReleasePath() {
  if (CORTEX_VARIANT == file_manager_utils::kNightlyVariant) {
    return "/cortex/latest/version.json";
  } else if (CORTEX_VARIANT == file_manager_utils::kBetaVariant) {
    return "/repos/janhq/cortex.cpp/releases";
  } else {
    return "/repos/janhq/cortex.cpp/releases/latest";
  }
}

inline void CheckNewUpdate() {
  auto host_name = GetHostName();
  auto release_path = GetReleasePath();
  CTL_INF("Engine release path: " << host_name << release_path);

  httplib::Client cli(host_name);
  cli.set_connection_timeout(kTimeoutCheckUpdate);
  if (auto res = cli.Get(release_path)) {
    if (res->status == httplib::StatusCode::OK_200) {
      try {
        auto get_latest = [](const nlohmann::json& data) -> std::string {
          if (data.empty()) {
            return "";
          }

          if (CORTEX_VARIANT == file_manager_utils::kBetaVariant) {
            for (auto& d : data) {
              if (auto tag = d["tag_name"].get<std::string>();
                  tag.find(kBetaComp) != std::string::npos) {
                return tag;
              }
            }
            return data[0]["tag_name"].get<std::string>();
          } else {
            return data["tag_name"].get<std::string>();
          }
          return "";
        };

        auto json_res = nlohmann::json::parse(res->body);
        std::string latest_version = get_latest(json_res);
        if (latest_version.empty()) {
          CTL_WRN("Release not found!");
          return;
        }
        std::string current_version = CORTEX_CPP_VERSION;
        if (current_version != latest_version) {
          CLI_LOG("\nA new release of cortex is available: "
                  << current_version << " -> " << latest_version);
          CLI_LOG("To upgrade, run: " << GetRole() << GetCortexBinary()
                                      << " update");
          if (CORTEX_VARIANT == file_manager_utils::kProdVariant) {
            CLI_LOG(json_res["html_url"].get<std::string>());
          }
        }
      } catch (const nlohmann::json::parse_error& e) {
        CTL_INF("JSON parse error: " << e.what());
      }
    } else {
      CTL_INF("HTTP error: " << res->status);
    }
  } else {
    auto err = res.error();
    CTL_INF("HTTP error: " << httplib::to_string(err));
  }
}

inline bool ReplaceBinaryInflight(const std::filesystem::path& src,
                                  const std::filesystem::path& dst) {
  if (src == dst) {
    // Already has the newest
    return true;
  }

  std::filesystem::path temp = dst.parent_path() / "cortex_temp";
  auto restore_binary = [&temp, &dst]() {
    if (std::filesystem::exists(temp)) {
      std::rename(temp.string().c_str(), dst.string().c_str());
      CLI_LOG("Restored binary file");
    }
  };

  try {
    if (std::filesystem::exists(temp)) {
      std::filesystem::remove(temp);
    }
#if !defined(_WIN32)
    // Get permissions of the executable file
    struct stat dst_file_stat;
    if (stat(dst.string().c_str(), &dst_file_stat) != 0) {
      CTL_ERR("Error getting permissions of executable file: " << dst.string());
      return false;
    }

    // Get owner and group of the executable file
    uid_t dst_file_owner = dst_file_stat.st_uid;
    gid_t dst_file_group = dst_file_stat.st_gid;
#endif

    std::rename(dst.string().c_str(), temp.string().c_str());
    std::filesystem::copy_file(
        src, dst, std::filesystem::copy_options::overwrite_existing);

#if !defined(_WIN32)
    // Set permissions of the executable file
    if (chmod(dst.string().c_str(), dst_file_stat.st_mode) != 0) {
      CTL_ERR("Error setting permissions of executable file: " << dst.string());
      restore_binary();
      return false;
    }

    // Set owner and group of the executable file
    if (chown(dst.string().c_str(), dst_file_owner, dst_file_group) != 0) {
      CTL_ERR(
          "Error setting owner and group of executable file: " << dst.string());
      restore_binary();
      return false;
    }

    // Remove cortex_temp
    if (unlink(temp.string().c_str()) != 0) {
      CTL_ERR("Error deleting self: " << strerror(errno));
      restore_binary();
      return false;
    }
#endif
  } catch (const std::exception& e) {
    CTL_ERR("Something went wrong: " << e.what());
    restore_binary();
    return false;
  }

  return true;
}

// This class manages the 'cortex update' command functionality
// There are three release types available:
// - Stable: Only retrieves the latest version
// - Beta: Allows retrieval of the latest beta version and specific versions using the -v flag
// - Nightly: Enables retrieval of the latest nightly build and specific versions using the -v flag
class CortexUpdCmd {
 public:
  void Exec(std::string version);

 private:
  bool GetStable(const std::string& v);
  bool GetBeta(const std::string& v);
  bool HandleGithubRelease(const nlohmann::json& assets,
                           const std::string& os_arch);
  bool GetNightly(const std::string& v);
};
}  // namespace commands
