#pragma once
#include <optional>
#include <string>

#include "httplib.h"
#include "nlohmann/json.hpp"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace commands {
#ifndef CORTEX_VARIANT
#define CORTEX_VARIANT file_manager_utils::kProdVariant
#endif
constexpr const auto kNightlyHost = "https://delta.jan.ai";
constexpr const auto kNightlyFileName = "cortex-nightly.tar.gz";
const std::string kCortexBinary = "cortex";
constexpr const auto kBetaComp = "-rc";

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
        if(latest_version.empty()) {
          CTL_WRN("Release not found!");
          return;
        }
        std::string current_version = CORTEX_CPP_VERSION;
        if (current_version != latest_version) {
          CLI_LOG("\nA new release of cortex is available: "
                  << current_version << " -> " << latest_version);
          CLI_LOG("To upgrade, run: cortex update");
          // CLI_LOG(json_res["html_url"].get<std::string>());
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
  std::filesystem::path temp =
      std::filesystem::temp_directory_path() / "cortex_temp";

  try {
    if (std::filesystem::exists(temp)) {
      std::filesystem::remove(temp);
    }

    std::rename(dst.string().c_str(), temp.string().c_str());
    std::filesystem::copy_file(
        src, dst, std::filesystem::copy_options::overwrite_existing);
    std::filesystem::permissions(dst, std::filesystem::perms::owner_all |
                                          std::filesystem::perms::group_all |
                                          std::filesystem::perms::others_read |
                                          std::filesystem::perms::others_exec);
    auto download_folder = src.parent_path();
    std::filesystem::remove_all(download_folder);
  } catch (const std::exception& e) {
    CTL_ERR("Something wrong happened: " << e.what());
    if (std::filesystem::exists(temp)) {
      std::rename(temp.string().c_str(), dst.string().c_str());
      CLI_LOG("Restored binary file");
    }
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
  CortexUpdCmd();
  void Exec(std::string version);

 private:
  bool GetStable(const std::string& v);
  bool GetBeta(const std::string& v);
  bool HandleGithubRelease(const nlohmann::json& assets,
                           const std::string& os_arch);
  bool GetNightly(const std::string& v);
};

}  // namespace commands