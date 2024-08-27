#include "engine_init_cmd.h"
#include <utility>
#include "services/download_service.h"
#include "trantor/utils/Logger.h"
// clang-format off
#include "utils/cortexso_parser.h" 
#include "utils/archive_utils.h"   
#include "utils/system_info_utils.h"
// clang-format on

namespace commands {

EngineInitCmd::EngineInitCmd(std::string engineName, std::string version)
    : engineName_(std::move(engineName)), version_(std::move(version)) {}

void EngineInitCmd::Exec() const {
  if (engineName_.empty()) {
    LOG_ERROR << "Engine name is required";
    return;
  }

  // Check if the architecture and OS are supported
  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info.arch == system_info_utils::kUnsupported ||
      system_info.os == system_info_utils::kUnsupported) {
    LOG_ERROR << "Unsupported OS or architecture: " << system_info.os << ", "
              << system_info.arch;
    return;
  }

  // check if engine is supported
  if (std::find(supportedEngines_.begin(), supportedEngines_.end(),
                engineName_) == supportedEngines_.end()) {
    LOG_ERROR << "Engine not supported";
    return;
  }

  constexpr auto gitHubHost = "https://api.github.com";

  std::ostringstream engineReleasePath;
  engineReleasePath << "/repos/janhq/" << engineName_ << "/releases/"
                    << version_;

  using namespace nlohmann;

  httplib::Client cli(gitHubHost);
  if (auto res = cli.Get(engineReleasePath.str())) {
    if (res->status == httplib::StatusCode::OK_200) {
      try {
        auto jsonResponse = json::parse(res->body);
        auto assets = jsonResponse["assets"];
        auto os_arch{system_info.os + "-" + system_info.arch};

        for (auto& asset : assets) {
          auto assetName = asset["name"].get<std::string>();
          if (assetName.find(os_arch) != std::string::npos) {
            std::string host{"https://github.com"};

            auto full_url = asset["browser_download_url"].get<std::string>();
            std::string path = full_url.substr(host.length());

            auto fileName = asset["name"].get<std::string>();
            LOG_INFO << "URL: " << full_url;

            auto downloadTask = DownloadTask{.id = engineName_,
                                             .type = DownloadType::Engine,
                                             .error = std::nullopt,
                                             .items = {DownloadItem{
                                                 .id = engineName_,
                                                 .host = host,
                                                 .fileName = fileName,
                                                 .type = DownloadType::Engine,
                                                 .path = path,
                                             }}};

            DownloadService().AddDownloadTask(
                downloadTask,
                [&downloadTask](const std::string& absolute_path) {
                  // try to unzip the downloaded file
                  std::filesystem::path downloadedEnginePath{absolute_path};
                  LOG_INFO << "Downloaded engine path: "
                           << downloadedEnginePath.string();

                  archive_utils::ExtractArchive(
                      downloadedEnginePath.string(),
                      downloadedEnginePath.parent_path()
                          .parent_path()
                          .string());

                  // remove the downloaded file
                  std::filesystem::remove(absolute_path);
                  LOG_INFO << "Finished!";
                });

            return;
          }
        }
        LOG_ERROR << "No asset found for " << os_arch;
      } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
      }
    }
  } else {
    auto err = res.error();
    LOG_ERROR << "HTTP error: " << httplib::to_string(err);
  }
}

};  // namespace commands