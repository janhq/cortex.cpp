#include <trantor/utils/Logger.h>
#include <sstream>
#include <string>
#include <vector>

#include <services/download_service.h>
#include <nlohmann/json.hpp>
#include "httplib.h"

namespace cortexso_parser {
constexpr static auto kHuggingFaceHost = "https://huggingface.co";

inline std::optional<DownloadTask> getDownloadTask(
    const std::string& modelId, const std::string& branch = "main") {
  using namespace nlohmann;
  std::ostringstream oss;
  oss << "/api/models/cortexso/" << modelId << "/tree/" << branch;
  const std::string url = oss.str();

  std::ostringstream repoAndModelId;
  repoAndModelId << "cortexso/" << modelId;
  const std::string repoAndModelIdStr = repoAndModelId.str();

  httplib::Client cli(kHuggingFaceHost);
  if (auto res = cli.Get(url)) {
    if (res->status == httplib::StatusCode::OK_200) {
      try {
        auto jsonResponse = json::parse(res->body);

        std::vector<DownloadItem> downloadItems{};
        for (auto& [key, value] : jsonResponse.items()) {
          std::ostringstream downloadUrlOutput;
          auto path = value["path"].get<std::string>();
          downloadUrlOutput << repoAndModelIdStr << "/resolve/" << branch << "/"
                            << path;
          const std::string downloadUrl = downloadUrlOutput.str();

          DownloadItem downloadItem{};
          downloadItem.id = path;
          downloadItem.host = kHuggingFaceHost;
          downloadItem.fileName = path;
          downloadItem.type = DownloadType::Model;
          downloadItem.path = downloadUrl;
          downloadItem.totalSize = value["size"].get<int>();
          downloadItem.transferredSize = 0;
          downloadItem.status = DownloadStatus::Pending;
          downloadItems.push_back(downloadItem);
        }

        DownloadTask downloadTask{};
        downloadTask.id = modelId;
        downloadTask.type = DownloadType::Model;
        downloadTask.percentage = 0.0f;
        downloadTask.status = DownloadStatus::Pending;
        downloadTask.error = std::nullopt;
        downloadTask.items = downloadItems;

        return downloadTask;
      } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
      }
    }
  } else {
    auto err = res.error();
    LOG_ERROR << "HTTP error: " << httplib::to_string(err);
  }
  return std::nullopt;
}
}  // namespace cortexso_parser