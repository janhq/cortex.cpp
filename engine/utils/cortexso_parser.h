#include <trantor/utils/Logger.h>
#include <sstream>
#include <string>
#include <vector>

#include <services/download_service.h>
#include <nlohmann/json.hpp>
#include "httplib.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

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
        std::filesystem::path model_container_path =
            file_manager_utils::GetModelsContainerPath() / modelId;
        file_manager_utils::CreateDirectoryRecursively(
            model_container_path.string());

        for (const auto& [key, value] : jsonResponse.items()) {
          std::ostringstream downloadUrlOutput;
          auto path = value["path"].get<std::string>();
          if (path == ".gitattributes" || path == ".gitignore" ||
              path == "README.md") {
            continue;
          }
          downloadUrlOutput << kHuggingFaceHost << "/" << repoAndModelIdStr
                            << "/resolve/" << branch << "/" << path;
          const std::string download_url = downloadUrlOutput.str();
          auto local_path = model_container_path / path;

          downloadItems.push_back(DownloadItem{.id = path,
                                               .downloadUrl = download_url,
                                               .localPath = local_path});
        }

        DownloadTask downloadTask{
            .id = branch == "main" ? modelId : modelId + "-" + branch,
            .type = DownloadType::Model,
            .items = downloadItems};

        return downloadTask;
      } catch (const json::parse_error& e) {
        CTL_ERR("JSON parse error: {}" << e.what());
      }
    }
  } else {
    auto err = res.error();
    LOG_ERROR << "HTTP error: " << httplib::to_string(err);
  }
  return std::nullopt;
}
}  // namespace cortexso_parser
