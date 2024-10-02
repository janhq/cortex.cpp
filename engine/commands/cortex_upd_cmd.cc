#include "cortex_upd_cmd.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "server_stop_cmd.h"
#include "services/download_service.h"
#include "utils/archive_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/scope_exit.h"
#include "utils/system_info_utils.h"
#include "utils/url_parser.h"

namespace commands {

namespace {
std::chrono::seconds GetUpdateIntervalCheck() {
  if (CORTEX_VARIANT == file_manager_utils::kNightlyVariant) {
    return std::chrono::seconds(10 * 60);
  } else if (CORTEX_VARIANT == file_manager_utils::kBetaVariant) {
    return std::chrono::seconds(60 * 60);
  } else {
    return std::chrono::seconds(24 * 60 * 60);
  }
}

std::chrono::seconds GetTimeSinceEpochMillisec() {
  using namespace std::chrono;
  return duration_cast<seconds>(system_clock::now().time_since_epoch());
}
}  // namespace

std::optional<std::string> CheckNewUpdate(
    std::optional<std::chrono::milliseconds> timeout) {
  // Get info from .cortexrc
  auto should_check_update = false;
  auto config = file_manager_utils::GetCortexConfig();
  auto now = GetTimeSinceEpochMillisec();
  if (auto t = now - std::chrono::seconds(config.checkedForUpdateAt);
      t > GetUpdateIntervalCheck()) {
    should_check_update = true;
    config.checkedForUpdateAt = now.count();
    CTL_INF("Will check for new update, time from last check: " << t.count()
                                                                << " seconds");
  }

  if (!should_check_update) {
    CTL_INF("Will not check for new update, return the cache latest: "
            << config.latestRelease);
    return config.latestRelease;
  }

  auto host_name = GetHostName();
  auto release_path = GetReleasePath();
  CTL_INF("Engine release path: " << host_name << release_path);

  httplib::Client cli(host_name);
  if (timeout.has_value()) {
    cli.set_connection_timeout(*timeout);
    cli.set_read_timeout(*timeout);
  }
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
          return std::nullopt;
        }
        std::string current_version = CORTEX_CPP_VERSION;
        CTL_INF("Got the latest release, update to the config file: "
                << latest_version)
        config.latestRelease = latest_version;
        config_yaml_utils::DumpYamlConfig(
            config, file_manager_utils::GetConfigurationPath().string());
        if (current_version != latest_version) {
          return latest_version;
        }
      } catch (const nlohmann::json::parse_error& e) {
        CTL_INF("JSON parse error: " << e.what());
        return std::nullopt;
      }
    } else {
      CTL_INF("HTTP error: " << res->status);
      return std::nullopt;
    }
  } else {
    auto err = res.error();
    CTL_INF("HTTP error: " << httplib::to_string(err));
    return std::nullopt;
  }
  return std::nullopt;
}

bool ReplaceBinaryInflight(const std::filesystem::path& src,
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

void CortexUpdCmd::Exec(const std::string& v) {
  // Check for update, if current version is the latest, notify to user
  if (auto latest_version = commands::CheckNewUpdate(std::nullopt);
      latest_version.has_value() && *latest_version == CORTEX_CPP_VERSION) {
    CLI_LOG("cortex is up to date");
    return;
  }

  {
    auto config = file_manager_utils::GetCortexConfig();
    httplib::Client cli(config.apiServerHost + ":" + config.apiServerPort);
    auto res = cli.Get("/healthz");
    if (res) {
      CLI_LOG("Server is running. Stopping server before updating!");
      commands::ServerStopCmd ssc(config.apiServerHost,
                                  std::stoi(config.apiServerPort));
      ssc.Exec();
    }
  }

  // Try to remove cortex temp folder if it exists first
  try {
    auto n = std::filesystem::remove_all(
        std::filesystem::temp_directory_path() / "cortex");
    CTL_INF("Deleted " << n << " files or directories");
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
  }

  if (CORTEX_VARIANT == file_manager_utils::kProdVariant) {
    if (!GetStable(v))
      return;
  } else if (CORTEX_VARIANT == file_manager_utils::kBetaVariant) {
    if (!GetBeta(v))
      return;
  } else {
    if (!GetNightly(v))
      return;
  }
  CLI_LOG("Update cortex sucessfully");
}

bool CortexUpdCmd::GetStable(const std::string& v) {
  auto system_info = system_info_utils::GetSystemInfo();
  CTL_INF("OS: " << system_info->os << ", Arch: " << system_info->arch);

  // Download file
  auto github_host = GetHostName();
  auto release_path = GetReleasePath();
  CTL_INF("Engine release path: " << github_host << release_path);

  httplib::Client cli(github_host);
  if (auto res = cli.Get(release_path)) {
    if (res->status == httplib::StatusCode::OK_200) {
      try {
        auto json_data = nlohmann::json::parse(res->body);
        if (json_data.empty()) {
          CLI_LOG("Version not found: " << v);
          return false;
        }

        if (!HandleGithubRelease(json_data["assets"],
                                 {system_info->os + "-" + system_info->arch})) {
          return false;
        }
      } catch (const nlohmann::json::parse_error& e) {
        CTL_ERR("JSON parse error: " << e.what());
        return false;
      }
    } else {
      CTL_ERR("HTTP error: " << res->status);
      return false;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return false;
  }

  // Replace binary file
  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto src =
      std::filesystem::temp_directory_path() / "cortex" / GetCortexBinary();
  auto dst = executable_path / GetCortexBinary();
  utils::ScopeExit se([]() {
    auto cortex_tmp = std::filesystem::temp_directory_path() / "cortex";
    try {
      auto n = std::filesystem::remove_all(cortex_tmp);
      CTL_INF("Deleted " << n << " files or directories");
    } catch (const std::exception& e) {
      CTL_WRN(e.what());
    }
  });
  return ReplaceBinaryInflight(src, dst);
}

bool CortexUpdCmd::GetBeta(const std::string& v) {
  auto system_info = system_info_utils::GetSystemInfo();
  CTL_INF("OS: " << system_info->os << ", Arch: " << system_info->arch);

  // Download file
  auto github_host = GetHostName();
  auto release_path = GetReleasePath();
  CTL_INF("Engine release path: " << github_host << release_path);

  httplib::Client cli(github_host);
  if (auto res = cli.Get(release_path)) {
    if (res->status == httplib::StatusCode::OK_200) {
      try {
        auto json_res = nlohmann::json::parse(res->body);

        nlohmann::json json_data;
        for (auto& jr : json_res) {
          // Get the latest beta or match version
          if (auto tag = jr["tag_name"].get<std::string>();
              (v.empty() && tag.find(kBetaComp) != std::string::npos) ||
              (tag == v)) {
            json_data = jr;
            break;
          }
        }

        if (json_data.empty()) {
          CLI_LOG("Version not found: " << v);
          return false;
        }

        if (!HandleGithubRelease(json_data["assets"],
                                 {system_info->os + "-" + system_info->arch})) {
          return false;
        }
      } catch (const nlohmann::json::parse_error& e) {
        CTL_ERR("JSON parse error: " << e.what());
        return false;
      }
    } else {
      CTL_ERR("HTTP error: " << res->status);
      return false;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return false;
  }

  // Replace binary file
  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto src =
      std::filesystem::temp_directory_path() / "cortex" / GetCortexBinary();
  auto dst = executable_path / GetCortexBinary();
  utils::ScopeExit se([]() {
    auto cortex_tmp = std::filesystem::temp_directory_path() / "cortex";
    try {
      auto n = std::filesystem::remove_all(cortex_tmp);
      CTL_INF("Deleted " << n << " files or directories");
    } catch (const std::exception& e) {
      CTL_WRN(e.what());
    }
  });
  return ReplaceBinaryInflight(src, dst);
}

bool CortexUpdCmd::HandleGithubRelease(const nlohmann::json& assets,
                                       const std::string& os_arch) {
  std::string matched_variant = "";
  for (auto& asset : assets) {
    auto asset_name = asset["name"].get<std::string>();
    if (asset_name.find(kCortexBinary) != std::string::npos &&
        asset_name.find(os_arch) != std::string::npos &&
        asset_name.find(kReleaseFormat) != std::string::npos) {
      matched_variant = asset_name;
      break;
    }
    CTL_INF(asset_name);
  }
  if (matched_variant.empty()) {
    CTL_ERR("No variant found for " << os_arch);
    return false;
  }
  CTL_INF("Matched variant: " << matched_variant);

  for (auto& asset : assets) {
    auto asset_name = asset["name"].get<std::string>();
    if (asset_name == matched_variant) {
      auto download_url = asset["browser_download_url"].get<std::string>();
      auto file_name = asset["name"].get<std::string>();
      CTL_INF("Download url: " << download_url);

      auto local_path =
          std::filesystem::temp_directory_path() / "cortex" / asset_name;
      try {
        if (!std::filesystem::exists(local_path.parent_path())) {
          std::filesystem::create_directories(local_path.parent_path());
        }
      } catch (const std::filesystem::filesystem_error& e) {
        CTL_ERR("Failed to create directories: " << e.what());
        return false;
      }
      auto download_task{DownloadTask{.id = "cortex",
                                      .type = DownloadType::Cortex,
                                      .items = {DownloadItem{
                                          .id = "cortex",
                                          .downloadUrl = download_url,
                                          .localPath = local_path,
                                      }}}};

      DownloadService().AddDownloadTask(
          download_task, [](const DownloadTask& finishedTask) {
            // try to unzip the downloaded file
            CTL_INF("Downloaded engine path: "
                    << finishedTask.items[0].localPath.string());

            auto extract_path =
                finishedTask.items[0].localPath.parent_path().parent_path();

            archive_utils::ExtractArchive(
                finishedTask.items[0].localPath.string(),
                extract_path.string());

            CTL_INF("Finished!");
          });
      break;
    }
  }
  return true;
}

bool CortexUpdCmd::GetNightly(const std::string& v) {
  auto system_info = system_info_utils::GetSystemInfo();
  CTL_INF("OS: " << system_info->os << ", Arch: " << system_info->arch);

  // Download file
  std::string version = v.empty() ? "latest" : std::move(v);
  std::string os_arch{system_info->os + "-" + system_info->arch};
  const char* paths[] = {
      "cortex",
      version.c_str(),
      os_arch.c_str(),
      kNightlyFileName,
  };
  std::vector<std::string> path_list(paths, std::end(paths));
  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = kNightlyHost,
      .pathParams = path_list,
  };

  CTL_INF("Engine release path: " << url_parser::FromUrl(url_obj));

  std::filesystem::path localPath =
      std::filesystem::temp_directory_path() / "cortex" / path_list.back();
  try {
    if (!std::filesystem::exists(localPath.parent_path())) {
      std::filesystem::create_directories(localPath.parent_path());
    }
  } catch (const std::filesystem::filesystem_error& e) {
    CTL_ERR("Failed to create directories: " << e.what());
    return false;
  }
  auto download_task =
      DownloadTask{.id = "cortex",
                   .type = DownloadType::Cortex,
                   .items = {DownloadItem{
                       .id = "cortex",
                       .downloadUrl = url_parser::FromUrl(url_obj),
                       .localPath = localPath,
                   }}};

  auto res = DownloadService().AddDownloadTask(
      download_task, [](const DownloadTask& finishedTask) {
        // try to unzip the downloaded file
        CTL_INF("Downloaded engine path: "
                << finishedTask.items[0].localPath.string());

        auto extract_path =
            finishedTask.items[0].localPath.parent_path().parent_path();

        archive_utils::ExtractArchive(finishedTask.items[0].localPath.string(),
                                      extract_path.string());

        CTL_INF("Finished!");
      });

  if (res.has_error()) {
    CLI_LOG("Download failed: " << res.error());
    return false;
  }

  // Replace binary file
  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto src =
      std::filesystem::temp_directory_path() / "cortex" / GetCortexBinary();
  auto dst = executable_path / GetCortexBinary();
  utils::ScopeExit se([]() {
    auto cortex_tmp = std::filesystem::temp_directory_path() / "cortex";
    try {
      auto n = std::filesystem::remove_all(cortex_tmp);
      CTL_INF("Deleted " << n << " files or directories");
    } catch (const std::exception& e) {
      CTL_WRN(e.what());
    }
  });
  return ReplaceBinaryInflight(src, dst);
}
}  // namespace commands
