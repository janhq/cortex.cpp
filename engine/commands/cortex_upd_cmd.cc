// clang-format off
#include "utils/cortex_utils.h"
// clang-format on
#include "cortex_upd_cmd.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "services/download_service.h"
#include "utils/archive_utils.h"
#include "utils/logging_utils.h"
#include "utils/system_info_utils.h"
#if defined(_WIN32) || defined(__linux__)
#include "utils/file_manager_utils.h"
#endif

namespace commands {

namespace {
    const std::string kCortexBinary = "cortex-cpp";
}   

CortexUpdCmd::CortexUpdCmd() {}

void CortexUpdCmd::Exec() {
  // Check if the architecture and OS are supported
  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info.arch == system_info_utils::kUnsupported ||
      system_info.os == system_info_utils::kUnsupported) {
    CTL_ERR("Unsupported OS or architecture: " << system_info.os << ", "
                                               << system_info.arch);
    return;
  }
  CTL_INF("OS: " << system_info.os << ", Arch: " << system_info.arch);

  // Download file
  constexpr auto github_host = "https://api.github.com";
  //   std::string version = version_.empty() ? "latest" : version_;
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
          if (asset_name.find("cortex-cpp") != std::string::npos &&
              asset_name.find(os_arch) != std::string::npos) {
            matched_variant = asset_name;
            break;
          }
          CTL_INF(asset_name);
        }
        if (matched_variant.empty()) {
          CTL_ERR("No variant found for " << os_arch);
          return;
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

                  // remove the downloaded file
                  // TODO(any) Could not delete file on Windows because it is currently hold by httplib(?)
                  // Not sure about other platforms
                  try {
                    std::filesystem::remove(absolute_path);
                  } catch (const std::exception& e) {
                    CTL_WRN("Could not delete file: " << e.what());
                  }
                  CTL_INF("Finished!");
                });
            break;
          }
        }
      } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return;
      }
    } else {
      CTL_ERR("HTTP error: " << res->status);
      return;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return;
  }
#if defined(_WIN32)
  std::string temp = ".\\cortex_tmp.exe";
  remove(temp.c_str());  // ignore return code

  std::string src = ".\\Cortex\\" + kCortexBinary + "\\" + kCortexBinary + ".exe";
  std::string dst = ".\\" + kCortexBinary + ".exe";
  // Rename
  rename(dst.c_str(), temp.c_str());
  // Update
  CopyFile(const_cast<char*>(src.c_str()), const_cast<char*>(dst.c_str()),
           false);
#else
  std::string src = "./Cortex/" + kCortexBinary + "/" + kCortexBinary;
  std::string dst = "./" + kCortexBinary;
  std::filesystem::copy_file(src, dst,
                             std::filesystem::copy_options::overwrite_existing);
#endif
  CLI_LOG("Update cortex sucessfully");
}
}  // namespace commands