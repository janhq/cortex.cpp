#include "engines.h"
#include "utils/archive_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/system_info_utils.h"

void Engines::InitEngine(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& engine) const {
  LOG_DEBUG << "InitEngine, Engine: " << engine;
  if (engine.empty()) {
    Json::Value res;
    res["message"] = "Engine name is required";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "No engine field in path param";
    return;
  }

  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info.arch == system_info_utils::kUnsupported ||
      system_info.os == system_info_utils::kUnsupported) {
    Json::Value res;
    res["message"] = "Unsupported OS or architecture";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_ERROR << "Unsupported OS or architecture: " << system_info.os << ", "
              << system_info.arch;
    return;
  }

  auto version{"latest"};
  constexpr auto gitHubHost = "https://api.github.com";

  std::ostringstream engineReleasePath;
  engineReleasePath << "/repos/janhq/" << engine << "/releases/" << version;

  httplib::Client cli(gitHubHost);
  using namespace nlohmann;
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

            auto downloadTask = DownloadTask{.id = engine,
                                             .type = DownloadType::Engine,
                                             .error = std::nullopt,
                                             .items = {DownloadItem{
                                                 .id = engine,
                                                 .host = host,
                                                 .fileName = fileName,
                                                 .type = DownloadType::Engine,
                                                 .path = path,
                                             }}};

            DownloadService().AddAsyncDownloadTask(
                downloadTask, [](const std::string& absolute_path) {
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

            Json::Value res;
            res["message"] = "Engine download started";
            res["result"] = "OK";
            auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
            resp->setStatusCode(k200OK);
            callback(resp);
            return;
          }
        }
        Json::Value res;
        res["message"] = "Engine not found";
        res["result"] = "Error";
        auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
        resp->setStatusCode(k404NotFound);
        callback(resp);
      } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
      }
    }
  } else {
    auto err = res.error();
    LOG_ERROR << "HTTP error: " << httplib::to_string(err);
  }
}