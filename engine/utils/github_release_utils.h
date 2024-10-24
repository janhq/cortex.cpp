#pragma once

#include <Security/cssmconfig.h>
#include <unordered_map>
#include "utils/huggingface_utils.h"
#include "utils/json_parser_utils.h"
#include "utils/result.hpp"
#include "utils/url_parser.h"

namespace github_release_utils {
struct GitHubAsset {
  std::string url;
  int id;
  std::string node_id;
  std::string name;
  std::string label;

  std::string content_type;
  std::string state;
  uint64 size;
  uint32 download_count;
  std::string created_at;

  std::string updated_at;
  std::string browser_download_url;
};

struct GitHubRelease {
  std::string url;
  int id;
  std::string tag_name;
  std::string name;
  bool draft;

  bool prerelease;
  std::string created_at;
  std::string published_at;
  std::vector<GitHubAsset> assets;

  static GitHubRelease FromJson(const Json::Value& json) {
    return GitHubRelease{
        .url = json["url"].asString(),
        .id = json["id"].asInt(),
        .tag_name = json["tag_name"].asString(),
        .name = json["name"].asString(),
        .draft = json["draft"].asBool(),
        .prerelease = json["prerelease"].asBool(),
        .created_at = json["created_at"].asString(),
        .published_at = json["published_at"].asString(),
        .assets =
            json_parser_utils::ParseJsonArray<GitHubAsset>(json["assets"]),
    };
  }
};

// TODO: (namh) support pagination for this api
// TODO: (namh) might need to add support for github token for better API through put
inline cpp::result<std::vector<GitHubRelease>, std::string> GetReleases(
    const std::string& author, const std::string& repo,
    const bool allow_prerelease = true) {
  auto url = url_parser::Url{
      .protocol = "https",
      .host = "api.github.com",
      .pathParams = {"repos", author, repo, "releases"},
  };

  auto result = curl_utils::SimpleGetJson(url_parser::FromUrl(url));

  if (result.has_error()) {
    return cpp::fail(result.error());
  }

  if (!result->isArray()) {
    return cpp::fail("Releases returned is not an array!");
  }

  return json_parser_utils::ParseJsonArray<GitHubRelease>(result.value());
}
};  // namespace github_release_utils
