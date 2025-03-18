#include "cortex_upd_cmd.h"
#include "cli/commands/server_start_cmd.h"
#include "server_stop_cmd.h"
#include "utils/archive_utils.h"
#include "utils/curl_utils.h"
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

std::unique_ptr<system_info_utils::SystemInfo> GetSystemInfoWithUniversal() {
  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info->os == "mac") {
    CTL_INF("Change arch from " << system_info->arch << " to universal");
    system_info->arch = "universal";
  }
  return system_info;
}

std::string GetNightlyInstallerName(const std::string& v,
                                    const std::string& os_arch) {
  const std::string kCortex = "cortex";
  // Remove 'v' in file name
  std::string version = v == "latest" ? "" : (v.substr(1) + "-");
#if defined(__APPLE__) && defined(__MACH__)
  return kCortex + "-" + version + os_arch + "-network-installer.pkg";
#elif defined(__linux__)
  return kCortex + "-" + version + os_arch + "-network-installer.deb";
#else
  return kCortex + "-" + version + os_arch + "-network-installer.exe";
#endif
}

std::string GetInstallCmd(const std::string& exe_path) {
#if defined(__APPLE__) && defined(__MACH__)
  return "sudo touch /var/tmp/cortex_installer_skip_postinstall_check && sudo "
         "installer "
         "-pkg " +
         exe_path +
         " -target / && sudo rm "
         "/var/tmp/cortex_installer_skip_postinstall_check";
#elif defined(__linux__)
  return "echo -e \"n\\n\" | sudo SKIP_POSTINSTALL=true apt install -y "
         "--allow-downgrades " +
         exe_path;
#else
  return "start /wait \"\" " + exe_path +
         " /VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SkipPostInstall";
#endif
}

std::string GetInstallCmdLinux(const std::string& script_path,
                               const std::string& channel,
                               const std::string& version) {
  std::string cmd = "sudo " + script_path;
  if (!channel.empty()) {
    cmd += " --channel " + channel;
  }
  if (!version.empty()) {
    cmd += " --version " + version.substr(1);
  }
  return cmd + " --is_update";
}

bool InstallNewVersion(const std::filesystem::path& dst,
                       const std::string& exe_script_path,
                       const std::string& channel, const std::string& version) {
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
    // rename binary
    std::rename(dst.string().c_str(), temp.string().c_str());
    // install here
    std::string install_cmd;
#if defined(__linux__)
    install_cmd = GetInstallCmdLinux(exe_script_path, channel, version);
#else
    install_cmd = GetInstallCmd(exe_script_path);
#endif
    CTL_INF("Cmd: " << install_cmd);
    CommandExecutor c(install_cmd);
    auto output = c.execute();
    if (!std::filesystem::exists(dst)) {
      CLI_LOG_ERROR("Something went wrong: could not execute command");
      restore_binary();
      return false;
    }

    // delete temp
#if !defined(_WIN32)
    if (unlink(temp.string().c_str()) != 0) {
      CLI_LOG_ERROR("Error deleting self: " << strerror(errno));
      restore_binary();
      return false;
    }
#endif

  } catch (const std::exception& e) {
    CLI_LOG_ERROR("Something went wrong: " << e.what());
    restore_binary();
    return false;
  }
  return true;
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

  auto url = url_parser::Url{
      .protocol = "https",
      .host = GetHostName(),
      .pathParams = GetReleasePath(),
  };

  CTL_INF("Engine release path: " << url.ToFullPath());

  auto res = curl_utils::SimpleGetJson(url.ToFullPath());
  if (res.has_error()) {
    CTL_INF("HTTP error: " << res.error());
    return std::nullopt;
  }

  try {
    auto get_latest = [](const Json::Value& data) -> std::string {
      if (data.empty()) {
        return "";
      }

      if (CORTEX_VARIANT == file_manager_utils::kBetaVariant) {
        for (const auto& d : data) {
          if (auto tag = d["tag_name"].asString();
              tag.find(kBetaComp) != std::string::npos) {
            return tag;
          }
        }
        return data[0]["tag_name"].asString();
      } else {
        return data["tag_name"].asString();
      }
      return "";
    };

    auto latest_version = get_latest(res.value());
    if (latest_version.empty()) {
      CTL_WRN("Release not found!");
      return std::nullopt;
    }
    std::string current_version = CORTEX_CPP_VERSION;
    CTL_INF(
        "Got the latest release, update to the config file: " << latest_version)
    config.latestRelease = latest_version;
    auto result =
        config_yaml_utils::CortexConfigMgr::GetInstance().DumpYamlConfig(
            config, file_manager_utils::GetConfigurationPath().string());
    if (result.has_error()) {
      CTL_ERR("Error update "
              << file_manager_utils::GetConfigurationPath().string()
              << result.error());
    }
    if (current_version != latest_version) {
      return latest_version;
    }
  } catch (const std::exception& e) {
    CTL_INF("JSON parse error: " << e.what());
    return std::nullopt;
  }
  return std::nullopt;
}

void CortexUpdCmd::Exec(const std::string& v, bool force) {
  // Check for update, if current version is the latest, notify to user
  if (auto latest_version = commands::CheckNewUpdate(std::nullopt);
      latest_version.has_value() && *latest_version == CORTEX_CPP_VERSION &&
      !force) {
    CLI_LOG("cortex is up to date");
    return;
  }

  {
    auto config = file_manager_utils::GetCortexConfig();
    auto server_running = commands::IsServerAlive(
        config.apiServerHost, std::stoi(config.apiServerPort));
    if (server_running) {
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
  CLI_LOG("Updated cortex successfully");
}

bool CortexUpdCmd::GetStable(const std::string& v) {
#if defined(__linux__)
  return GetLinuxInstallScript(v, "stable");
#else
  std::optional<std::string> downloaded_exe_path;
  auto system_info = GetSystemInfoWithUniversal();
  CTL_INF("OS: " << system_info->os << ", Arch: " << system_info->arch);

  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = GetHostName(),
      .pathParams = GetReleasePath(),
  };
  CTL_INF("Engine release path: " << url_obj.ToFullPath());

  auto res = curl_utils::SimpleGetJson(url_obj.ToFullPath());
  if (res.has_error()) {
    CLI_LOG_ERROR("HTTP error: " << res.error());
    return false;
  }

  try {
    if (res.value().empty()) {
      CLI_LOG("Version not found: " << v);
      return false;
    }

    if (downloaded_exe_path = HandleGithubRelease(
            res.value()["assets"], {system_info->os + "-" + system_info->arch});
        !downloaded_exe_path) {
      return false;
    }
  } catch (const std::exception& e) {
    CLI_LOG_ERROR("JSON parse error: " << e.what());
    return false;
  }

  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto dst = executable_path / GetCortexBinary();
  cortex::utils::ScopeExit se([]() {
    auto cortex_tmp = std::filesystem::temp_directory_path() / "cortex";
    try {
      auto n = std::filesystem::remove_all(cortex_tmp);
      CTL_INF("Deleted " << n << " files or directories");
    } catch (const std::exception& e) {
      CTL_WRN(e.what());
    }
  });

  assert(!!downloaded_exe_path);
  return InstallNewVersion(dst, downloaded_exe_path.value(), "", "");
#endif
}

bool CortexUpdCmd::GetBeta(const std::string& v) {
#if defined(__linux__)
  return GetLinuxInstallScript(v, "beta");
#else
  std::optional<std::string> downloaded_exe_path;
  auto system_info = GetSystemInfoWithUniversal();
  CTL_INF("OS: " << system_info->os << ", Arch: " << system_info->arch);

  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = GetHostName(),
      .pathParams = GetReleasePath(),
  };
  CTL_INF("Engine release path: " << url_obj.ToFullPath());
  auto res = curl_utils::SimpleGetJson(url_obj.ToFullPath());
  if (res.has_error()) {
    CLI_LOG_ERROR("HTTP error: " << res.error());
    return false;
  }

  try {
    Json::Value json_data;
    for (const auto& jr : res.value()) {
      // Get the latest beta or match version
      if (auto tag = jr["tag_name"].asString();
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

    if (downloaded_exe_path = HandleGithubRelease(
            json_data["assets"], {system_info->os + "-" + system_info->arch});
        !downloaded_exe_path) {
      return false;
    }
  } catch (const std::exception& e) {
    CLI_LOG_ERROR("JSON parse error: " << e.what());
    return false;
  }

  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto dst = executable_path / GetCortexBinary();
  cortex::utils::ScopeExit se([]() {
    auto cortex_tmp = std::filesystem::temp_directory_path() / "cortex";
    try {
      auto n = std::filesystem::remove_all(cortex_tmp);
      CTL_INF("Deleted " << n << " files or directories");
    } catch (const std::exception& e) {
      CTL_WRN(e.what());
    }
  });

  assert(!!downloaded_exe_path);
  return InstallNewVersion(dst, downloaded_exe_path.value(), "", "");
#endif
}

std::optional<std::string> CortexUpdCmd::HandleGithubRelease(
    const Json::Value& assets, const std::string& os_arch) {
  std::string matched_variant = "";
  for (const auto& asset : assets) {
    auto asset_name = asset["name"].asString();
    if (asset_name.find(kCortexBinary) != std::string::npos &&
        asset_name.find(os_arch) != std::string::npos &&
        asset_name.find(kReleaseFormat) != std::string::npos) {
      matched_variant = asset_name;
      break;
    }
    CTL_INF(asset_name);
  }
  if (matched_variant.empty()) {
    CLI_LOG_ERROR("No variant found for " << os_arch);
    return std::nullopt;
  }
  CTL_INF("Matched variant: " << matched_variant);

  for (const auto& asset : assets) {
    auto asset_name = asset["name"].asString();
    if (asset_name == matched_variant) {
      auto download_url = asset["browser_download_url"].asString();
      auto file_name = asset["name"].asString();
      CTL_INF("Download url: " << download_url);

      auto local_path =
          std::filesystem::temp_directory_path() / "cortex" / asset_name;
      try {
        if (!std::filesystem::exists(local_path.parent_path())) {
          std::filesystem::create_directories(local_path.parent_path());
        }
      } catch (const std::filesystem::filesystem_error& e) {
        CLI_LOG_ERROR("Failed to create directories: " << e.what());
        return std::nullopt;
      }
      auto download_task{DownloadTask{
          .id = "cortex",
          .type = DownloadType::Cortex,
          .items = {DownloadItem{
              .id = "cortex",
              .downloadUrl = download_url,
              .localPath = local_path,
          }},
      }};

      auto result = download_service_->AddDownloadTask(
          download_task, [](const DownloadTask& finishedTask) {
            // try to unzip the downloaded file
            CTL_INF("Downloaded engine path: "
                    << finishedTask.items[0].localPath.string());

            CTL_INF("Finished!");
          });
      if (result.has_error()) {
        CLI_LOG_ERROR("Failed to download: " << result.error());
        return std::nullopt;
      }
      return local_path.string();
    }
  }
  return std::nullopt;
}

bool CortexUpdCmd::GetNightly(const std::string& v) {
#if defined(__linux__)
  return GetLinuxInstallScript(v, "nightly");
#else
  auto system_info = GetSystemInfoWithUniversal();
  CTL_INF("OS: " << system_info->os << ", Arch: " << system_info->arch);

  // Download file
  std::string version = v.empty() ? "latest" : v;
  std::string os_arch{system_info->os + "-" + system_info->arch};
  std::string installer_name = GetNightlyInstallerName(version, os_arch);
  const char* paths[] = {
      "cortex",
      version.c_str(),
      os_arch.c_str(),
      installer_name.c_str(),
  };
  std::vector<std::string> path_list(paths, std::end(paths));
  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = kNightlyHost,
      .pathParams = path_list,
  };

  CTL_INF("Cortex release path: " << url_parser::FromUrl(url_obj));

  std::filesystem::path localPath =
      std::filesystem::temp_directory_path() / "cortex" / path_list.back();
  try {
    if (!std::filesystem::exists(localPath.parent_path())) {
      std::filesystem::create_directories(localPath.parent_path());
    }
  } catch (const std::filesystem::filesystem_error& e) {
    CLI_LOG_ERROR("Failed to create directories: " << e.what());
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

  auto result = download_service_->AddDownloadTask(
      download_task, [](const DownloadTask& finishedTask) {
        // try to unzip the downloaded file
        CTL_INF("Downloaded cortex path: "
                << finishedTask.items[0].localPath.string());

        CTL_INF("Finished!");
      });
  if (result.has_error()) {
    CLI_LOG_ERROR("Failed to download: " << result.error());
    return false;
  }

  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto dst = executable_path / GetCortexBinary();
  cortex::utils::ScopeExit se([]() {
    auto cortex_tmp = std::filesystem::temp_directory_path() / "cortex";
    try {
      auto n = std::filesystem::remove_all(cortex_tmp);
      CTL_INF("Deleted " << n << " files or directories");
    } catch (const std::exception& e) {
      CTL_WRN(e.what());
    }
  });

  return InstallNewVersion(dst, localPath.string(), "", "");
#endif
}

bool CortexUpdCmd::GetLinuxInstallScript(const std::string& v,
                                         const std::string& channel) {
  std::vector<std::string> path_list;
  if (channel == "nightly") {
    path_list = {"menloresearch",     "cortex.cpp", "dev",       "engine",
                 "templates", "linux",      "install.sh"};
  } else {
    path_list = {"menloresearch",     "cortex.cpp", "main",      "engine",
                 "templates", "linux",      "install.sh"};
  }
  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = "raw.githubusercontent.com",
      .pathParams = path_list,
  };

  CTL_INF("Linux installer script path: " << url_parser::FromUrl(url_obj));

  std::filesystem::path localPath =
      std::filesystem::temp_directory_path() / "cortex" / path_list.back();
  try {
    if (!std::filesystem::exists(localPath.parent_path())) {
      std::filesystem::create_directories(localPath.parent_path());
    }
  } catch (const std::filesystem::filesystem_error& e) {
    CLI_LOG_ERROR("Failed to create directories: " << e.what());
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

  auto result = download_service_->AddDownloadTask(
      download_task, [](const DownloadTask& finishedTask) {
        // try to unzip the downloaded file
        CTL_INF("Downloaded cortex path: "
                << finishedTask.items[0].localPath.string());

        CTL_INF("Finished!");
      });
  if (result.has_error()) {
    CLI_LOG_ERROR("Failed to download: " << result.error());
    return false;
  }

  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto dst = executable_path / GetCortexBinary();
  cortex::utils::ScopeExit se([]() {
    auto cortex_tmp = std::filesystem::temp_directory_path() / "cortex";
    try {
      auto n = std::filesystem::remove_all(cortex_tmp);
      CTL_INF("Deleted " << n << " files or directories");
    } catch (const std::exception& e) {
      CTL_WRN(e.what());
    }
  });
  try {
    std::filesystem::permissions(localPath,
                                 std::filesystem::perms::owner_exec |
                                     std::filesystem::perms::group_exec |
                                     std::filesystem::perms::others_exec,
                                 std::filesystem::perm_options::add);
  } catch (const std::filesystem::filesystem_error& e) {
    CTL_WRN("Error: " << e.what());
    return false;
  }

  return InstallNewVersion(dst, localPath.string(), channel, v);
}
}  // namespace commands
