#include "cortex_upd_cmd.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "server_stop_cmd.h"
#include "services/download_service.h"
#include "utils/archive_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/system_info_utils.h"
#include "utils/url_parser.h"

namespace commands {

void CortexUpdCmd::Exec(std::string v) {
  {
    auto config = file_manager_utils::GetCortexConfig();
    httplib::Client cli(config.apiServerHost + ":" + config.apiServerPort);
    auto res = cli.Get("/health/healthz");
    if (res) {
      CLI_LOG("Server is running. Stopping server before updating!");
      commands::ServerStopCmd ssc(config.apiServerHost,
                                  std::stoi(config.apiServerPort));
      ssc.Exec();
    }
  }
  if (CORTEX_VARIANT == file_manager_utils::kNightlyVariant) {
    if (!GetNightly(v))
      return;
  } else {
    if (!GetStableAndBeta(v))
      return;
  }
  CLI_LOG("Update cortex sucessfully");
}

bool CortexUpdCmd::GetStableAndBeta(const std::string& v) {
  auto system_info = system_info_utils::GetSystemInfo();
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
            auto download_url =
                asset["browser_download_url"].get<std::string>();
            auto file_name = asset["name"].get<std::string>();
            CTL_INF("Download url: " << download_url);

            auto local_path =
                file_manager_utils::GetExecutableFolderContainerPath() /
                "cortex" / asset_name;
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

                  auto extract_path = finishedTask.items[0]
                                          .localPath.parent_path()
                                          .parent_path();

                  archive_utils::ExtractArchive(
                      finishedTask.items[0].localPath.string(),
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

  // Replace binary file
  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto src = std::filesystem::temp_directory_path() / "cortex" / kCortexBinary /
             GetCortexBinary();
  auto dst = executable_path / GetCortexBinary();
  return ReplaceBinaryInflight(src, dst);
}

bool CortexUpdCmd::GetNightly(const std::string& v) {
  auto system_info = system_info_utils::GetSystemInfo();
  CTL_INF("OS: " << system_info.os << ", Arch: " << system_info.arch);

  // Download file
  std::string version = v.empty() ? "latest" : std::move(v);
  std::string os_arch{system_info.os + "-" + system_info.arch};
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
      file_manager_utils::GetExecutableFolderContainerPath() / "cortex" /
      path_list.back();
  auto download_task =
      DownloadTask{.id = "cortex",
                   .type = DownloadType::Cortex,
                   .items = {DownloadItem{
                       .id = "cortex",
                       .downloadUrl = url_parser::FromUrl(url_obj),
                       .localPath = localPath,
                   }}};

  DownloadService().AddDownloadTask(
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

  // Replace binary file
  auto executable_path = file_manager_utils::GetExecutableFolderContainerPath();
  auto src = std::filesystem::temp_directory_path() / "cortex" / kCortexBinary /
             GetCortexBinary();
  auto dst = executable_path / GetCortexBinary();
  return ReplaceBinaryInflight(src, dst);
}
}  // namespace commands
