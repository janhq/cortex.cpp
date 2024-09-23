#include <trantor/utils/Logger.h>
#include <string>
#include <vector>

#include <services/download_service.h>
#include <nlohmann/json.hpp>
#include "httplib.h"
#include "utils/file_manager_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"

namespace cortexso_parser {
constexpr static auto kHuggingFaceHost = "huggingface.co";

inline std::optional<DownloadTask> getDownloadTask(
    const std::string& modelId, const std::string& branch = "main") {
  using namespace nlohmann;
  url_parser::Url url = {
      .protocol = "https",
      .host = kHuggingFaceHost,
      .pathParams = {"api", "models", "cortexso", modelId, "tree", branch}};

  httplib::Client cli(url.GetProtocolAndHost());
  if (auto res = cli.Get(url.GetPathAndQuery())) {
    if (res->status == httplib::StatusCode::OK_200) {
      try {
        auto jsonResponse = json::parse(res->body);

        std::vector<DownloadItem> download_items{};
        auto model_container_path =
            file_manager_utils::GetModelsContainerPath() / "cortex.so" /
            modelId / branch;
        file_manager_utils::CreateDirectoryRecursively(
            model_container_path.string());

        for (const auto& [key, value] : jsonResponse.items()) {
          auto path = value["path"].get<std::string>();
          if (path == ".gitattributes" || path == ".gitignore" ||
              path == "README.md") {
            continue;
          }
          url_parser::Url download_url = {
              .protocol = "https",
              .host = kHuggingFaceHost,
              .pathParams = {"cortexso", modelId, "resolve", branch, path}};

          auto local_path = model_container_path / path;
          download_items.push_back(
              DownloadItem{.id = path,
                           .downloadUrl = download_url.ToFullPath(),
                           .localPath = local_path});
        }

        DownloadTask download_tasks{
            .id = branch == "main" ? modelId : modelId + "-" + branch,
            .type = DownloadType::Model,
            .items = download_items};

        return download_tasks;
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
