#pragma once

#include <json/value.h>
#include "utils/curl_utils.h"
#include "utils/engine_constants.h"
#include "utils/engine_matcher_utils.h"
#include "utils/logging_utils.h"
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
  uint64_t size;
  uint32_t download_count;
  std::string created_at;

  std::string updated_at;
  std::string browser_download_url;
  std::string version;

  static GitHubAsset FromJson(const Json::Value& json,
                              const std::string& version) {
    return GitHubAsset{
        .url = json["url"].asString(),
        .id = json["id"].asInt(),
        .node_id = json["node_id"].asString(),
        .name = json["name"].asString(),
        .label = json["label"].asString(),
        .content_type = json["content_type"].asString(),
        .state = json["state"].asString(),
        .size = json["size"].asUInt64(),
        .download_count = json["download_count"].asUInt(),
        .created_at = json["created_at"].asString(),
        .updated_at = json["updated_at"].asString(),
        .browser_download_url = json["browser_download_url"].asString(),
        .version = version,
    };
  }

  Json::Value ToJson() const {
    Json::Value root;
    root["url"] = url;
    root["id"] = id;
    root["node_id"] = node_id;
    root["name"] = name;
    root["label"] = label;
    root["content_type"] = content_type;
    root["state"] = state;
    root["size"] = size;
    root["download_count"] = download_count;
    root["created_at"] = created_at;
    root["updated_at"] = updated_at;
    root["browser_download_url"] = browser_download_url;
    root["version"] = version;
    return root;
  }

  std::optional<Json::Value> ToApiJson(const std::string& engine,
                                       const std::string& version) const {
    auto variant_name_result =
        engine_matcher_utils::GetVariantFromNameAndVersion(name, engine,
                                                           version);
    if (variant_name_result.has_error()) {
      CTL_ERR("Failed to get variant name: " << variant_name_result.error());
      return std::nullopt;
    }

    Json::Value root;
    root["name"] = variant_name_result.value();
    root["download_count"] = download_count;
    root["size"] = size;
    root["created_at"] = created_at;
    return root;
  }
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
    std::vector<GitHubAsset> assets = {};
    if (json["assets"].isArray()) {
      for (const auto& asset : json["assets"]) {
        assets.push_back(GitHubAsset::FromJson(asset, json["name"].asString()));
      }
    }

    return GitHubRelease{
        .url = json["url"].asString(),
        .id = json["id"].asInt(),
        .tag_name = json["tag_name"].asString(),
        .name = json["name"].asString(),
        .draft = json["draft"].asBool(),
        .prerelease = json["prerelease"].asBool(),
        .created_at = json["created_at"].asString(),
        .published_at = json["published_at"].asString(),
        .assets = assets,
    };
  }

  Json::Value ToApiJson() const {
    Json::Value root;
    root["url"] = url;
    root["name"] = tag_name;
    root["draft"] = draft;
    root["prerelease"] = prerelease;
    root["published_at"] = published_at;
    return root;
  }

  Json::Value ToJson() const {
    Json::Value assetsArray(Json::arrayValue);
    for (const auto& asset : assets) {
      assetsArray.append(asset.ToJson());
    }
    Json::Value root;
    root["url"] = url;
    root["id"] = id;
    root["tag_name"] = tag_name;
    root["name"] = name;
    root["draft"] = draft;
    root["prerelease"] = prerelease;
    root["created_at"] = created_at;
    root["published_at"] = published_at;
    root["assets"] = assetsArray;
    return root;
  }
};

// TODO: (namh) support pagination for this api
inline cpp::result<std::vector<GitHubRelease>, std::string> GetReleases(
    const std::string& author, const std::string& repo,
    const bool allow_prerelease = true) {
  auto url = url_parser::Url{
      .protocol = "https",
      .host = kGitHubHost,
      .pathParams = {"repos", author, repo, "releases"},
  };

  auto result = curl_utils::SimpleGetJson(url_parser::FromUrl(url));

  if (result.has_error()) {
    return cpp::fail(result.error());
  }

  if (!result.value().isArray()) {
    return cpp::fail("Releases returned is not an array!");
  }

  std::vector<GitHubRelease> releases{};
  for (const auto& release : result.value()) {
    releases.push_back(GitHubRelease::FromJson(release));
  }
  (void) allow_prerelease;
  return releases;
}

inline cpp::result<GitHubRelease, std::string> GetReleaseByVersion(
    const std::string& author, const std::string& repo,
    const std::string& tag) {

  std::vector<std::string> path_params{"repos", author, repo, "releases"};
  if (tag != "latest") {
    path_params.push_back("tags");

    // if (!string_utils::StartsWith(tag, "v")) {
    //   path_params.push_back("v" + tag);
    // }

    path_params.push_back(tag);
  } else {
    path_params.push_back("latest");
  }

  auto url = url_parser::Url{
      .protocol = "https",
      .host = kGitHubHost,
      .pathParams = path_params,
  };

  // CTL_DBG("GetReleaseByVersion: " << url.ToFullPath());
  auto result =
      curl_utils::SimpleGetJson(url_parser::FromUrl(url), kCurlGetTimeout);

  if (result.has_error()) {
    return cpp::fail(result.error());
  }
  if (result.value() == Json::nullValue) {
    return cpp::fail("No release found for " + tag);
  }

  return GitHubRelease::FromJson(result.value());
}
};  // namespace github_release_utils
