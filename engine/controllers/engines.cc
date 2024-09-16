#include "engines.h"
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <utility>
#include "services/engine_service.h"
#include "utils/archive_utils.h"
#include "utils/cortex_utils.h"
#include "utils/system_info_utils.h"

void Engines::InstallEngine(
    const HttpRequestPtr& req,
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
            auto download_url =
                asset["browser_download_url"].get<std::string>();
            auto name = asset["name"].get<std::string>();
            LOG_INFO << "Download url: " << download_url;

            std::filesystem::path engine_folder_path =
                file_manager_utils::GetContainerFolderPath(
                    file_manager_utils::DownloadTypeToString(
                        DownloadType::Engine)) /
                engine;

            if (!std::filesystem::exists(engine_folder_path)) {
              CTL_INF("Creating " << engine_folder_path.string());
              std::filesystem::create_directories(engine_folder_path);
            }
            auto local_path = engine_folder_path / assetName;
            auto downloadTask{DownloadTask{.id = engine,
                                           .type = DownloadType::Engine,
                                           .items = {DownloadItem{
                                               .id = engine,
                                               .downloadUrl = download_url,
                                               .localPath = local_path,
                                           }}}};

            DownloadService().AddAsyncDownloadTask(
                downloadTask, [](const DownloadTask& finishedTask) {
                  // try to unzip the downloaded file
                  archive_utils::ExtractArchive(
                      finishedTask.items[0].localPath.string(),
                      finishedTask.items[0]
                          .localPath.parent_path()
                          .parent_path()
                          .string());

                  // remove the downloaded file
                  try {
                    std::filesystem::remove(finishedTask.items[0].localPath);
                  } catch (const std::exception& e) {
                    LOG_WARN << "Could not delete file: " << e.what();
                  }
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

void Engines::ListEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  auto engine_service = EngineService();
  auto status_list = engine_service.GetEngineInfoList();

  Json::Value ret;
  ret["object"] = "list";
  Json::Value data(Json::arrayValue);
  for (auto& status : status_list) {
    Json::Value ret;
    ret["name"] = status.name;
    ret["description"] = status.description;
    ret["version"] = status.version;
    ret["productName"] = status.product_name;
    ret["status"] = status.status;

    data.append(std::move(ret));
  }

  ret["data"] = data;
  ret["result"] = "OK";
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Engines::GetEngine(const HttpRequestPtr& req,
                        std::function<void(const HttpResponsePtr&)>&& callback,
                        const std::string& engine) const {
  auto engine_service = EngineService();
  try {
    auto status = engine_service.GetEngineInfo(engine);
    Json::Value ret;
    ret["name"] = status.name;
    ret["description"] = status.description;
    ret["version"] = status.version;
    ret["productName"] = status.product_name;
    ret["status"] = status.status;

    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  } catch (const std::runtime_error e) {
    Json::Value ret;
    ret["message"] = e.what();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } catch (const std::exception& e) {
    Json::Value ret;
    ret["message"] = e.what();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k500InternalServerError);
    callback(resp);
  }
}

void Engines::UninstallEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) const {
  LOG_INFO << "[Http] Uninstall engine " << engine;
  auto engine_service = EngineService();

  Json::Value ret;
  try {
    // TODO: Unload the model which is currently running on engine_
    // TODO: Unload engine if is loaded
    engine_service.UninstallEngine(engine);

    ret["message"] = "Engine " + engine + " uninstalled successfully!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  } catch (const std::runtime_error& e) {
    CLI_LOG("Runtime exception");
    ret["message"] = "Engine " + engine + " is not installed!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } catch (const std::exception& e) {
    ret["message"] = "Engine " + engine + " failed to uninstall: " + e.what();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  }
}
