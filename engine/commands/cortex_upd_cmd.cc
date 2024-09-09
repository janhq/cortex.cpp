// clang-format off
#include "utils/cortex_utils.h"
// clang-format on
#include "cortex_upd_cmd.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "services/download_service.h"
#include "utils/archive_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/system_info_utils.h"
#include "server_stop_cmd.h"

namespace commands {

CortexUpdCmd::CortexUpdCmd() {}

void CortexUpdCmd::Exec(std::string v) {
  // TODO(sang) check if server is running - if yes, stop it
  // {
  //   commands::ServerStopCmd ssc("127.0.0.1", 3928);
  //   ssc.Exec();
  // }
  if (CORTEX_VARIANT == file_manager_utils::kNightlyVariant) {
    if (!GetNightly(v))
      return;
  } else {
    if (!GetProAndBeta(v))
      return;
  }
  CLI_LOG("Update cortex sucessfully");
}

bool CortexUpdCmd::GetProAndBeta(const std::string& v) {
  // Check if the architecture and OS are supported
  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info.arch == system_info_utils::kUnsupported ||
      system_info.os == system_info_utils::kUnsupported) {
    CTL_ERR("Unsupported OS or architecture: " << system_info.os << ", "
                                               << system_info.arch);
    return false;
  }
  CTL_INF("OS: " << system_info.os << ", Arch: " << system_info.arch);

  // Download file
  constexpr auto github_host = "https://api.github.com";
  //   std::string version = v.empty() ? "latest" : std::move(v);
  // TODO(sang): support download with version
  std::string version = "latest";
  std::ostringstream release_path;
  release_path << "/repos/janhq/cortex.cpp/releases/" << version;
  CTL_INF("Engine release path: " << github_host << release_path.str());

  httplib::Client cli(github_host);
  if (auto res = cli.Get(release_path.str())) {
    if (res->status == httplib::StatusCode::OK_200) {
      try {
        auto jsonResponse = nlohmann::json::parse(res->body);
        auto assets = jsonResponse["assets"];
        auto os_arch{system_info.os + "-" + system_info.arch};

        std::string matched_variant = "";
        for (auto& asset : assets) {
          auto asset_name = asset["name"].get<std::string>();
          if (asset_name.find(kCortexBinary) != std::string::npos &&
              asset_name.find(os_arch) != std::string::npos) {
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
            std::string host{"https://github.com"};

            auto full_url = asset["browser_download_url"].get<std::string>();
            std::string path = full_url.substr(host.length());

            auto fileName = asset["name"].get<std::string>();
            CTL_INF("URL: " << full_url);

            auto download_task = DownloadTask{.id = "cortex",
                                              .type = DownloadType::Cortex,
                                              .error = std::nullopt,
                                              .items = {DownloadItem{
                                                  .id = "cortex",
                                                  .host = host,
                                                  .fileName = fileName,
                                                  .type = DownloadType::Cortex,
                                                  .path = path,
                                              }}};

            DownloadService download_service;
            download_service.AddDownloadTask(
                download_task,
                [this](const std::string& absolute_path, bool unused) {
                  // try to unzip the downloaded file
                  std::filesystem::path download_path{absolute_path};
                  CTL_INF("Downloaded engine path: " << download_path.string());

                  std::filesystem::path extract_path =
                      download_path.parent_path().parent_path();

                  archive_utils::ExtractArchive(download_path.string(),
                                                extract_path.string());

                  CTL_INF("Finished!");
                });
            break;
          }
        }
      } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
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

// Replace binay file
  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto src = executable_path / "cortex" / "cortex-cpp" / GetCortexBinary();
  auto dst = executable_path / GetCortexBinary();
  return ReplaceBinaryInflight(src, dst);
}

bool CortexUpdCmd::GetNightly(const std::string& v) {
  // Check if the architecture and OS are supported
  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info.arch == system_info_utils::kUnsupported ||
      system_info.os == system_info_utils::kUnsupported) {
    CTL_ERR("Unsupported OS or architecture: " << system_info.os << ", "
                                               << system_info.arch);
    return false;
  }
  CTL_INF("OS: " << system_info.os << ", Arch: " << system_info.arch);

  // Download file
  std::string version = v.empty() ? "latest" : std::move(v);
  std::ostringstream release_path;
  release_path << "cortex/" << version << "/" << system_info.os << "-"
               << system_info.arch << "/" << kNightlyFileName;
  CTL_INF("Engine release path: " << kNightlyHost << release_path.str());

  auto download_task = DownloadTask{.id = "cortex",
                                    .type = DownloadType::Cortex,
                                    .error = std::nullopt,
                                    .items = {DownloadItem{
                                        .id = "cortex",
                                        .host = kNightlyHost,
                                        .fileName = kNightlyFileName,
                                        .type = DownloadType::Cortex,
                                        .path = release_path.str(),
                                    }}};

  DownloadService download_service;
  download_service.AddDownloadTask(download_task, [this](const std::string&
                                                             absolute_path,
                                                         bool unused) {
    // try to unzip the downloaded file
    std::filesystem::path download_path{absolute_path};
    CTL_INF("Downloaded engine path: " << download_path.string());

    std::filesystem::path extract_path =
        download_path.parent_path().parent_path();

    archive_utils::ExtractArchive(download_path.string(),
                                  extract_path.string());

    CTL_INF("Finished!");
  });

  // Replace binay file
  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto src = executable_path / "cortex" / GetCortexBinary();
  auto dst = executable_path / GetCortexBinary();
  return ReplaceBinaryInflight(src, dst);
}

}  // namespace commands