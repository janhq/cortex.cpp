#include <json/value.h>
#include <string>
#include "utils/github_release_utils.h"
#include "utils/logging_utils.h"

namespace json_parser_utils {

template <typename T>
T jsonToValue(const Json::Value& value);

template <>
std::string jsonToValue(const Json::Value& value) {
  return value.asString();
}

template <>
int jsonToValue(const Json::Value& value) {
  return value.asInt();
}

template <>
double jsonToValue(const Json::Value& value) {
  return value.asDouble();
}

template <>
bool jsonToValue(const Json::Value& value) {
  return value.asBool();
}

template <>
github_release_utils::GitHubAsset jsonToValue(const Json::Value& value) {
  return github_release_utils::GitHubAsset{
      .url = value["url"].asString(),
      .id = value["id"].asInt(),
      .node_id = value["node_id"].asString(),
      .name = value["name"].asString(),
      .label = value["label"].asString(),
      .content_type = value["content_type"].asString(),
      .state = value["state"].asString(),
      .size = value["size"].asUInt64(),
      .download_count = value["download_count"].asUInt(),
      .created_at = value["created_at"].asString(),
      .updated_at = value["updated_at"].asString(),
      .browser_download_url = value["browser_download_url"].asString(),
  };
}

template <typename T>
std::vector<T> ParseJsonArray(const Json::Value& array) {
  try {
    std::vector<T> result;
    if (array.isArray()) {
      result.reserve(array.size());
      for (const Json::Value& element : array) {
        result.push_back(jsonToValue<T>(element));
      }
    }
    return result;
  } catch (const std::exception& e) {
    CTL_ERR("Error parsing json array: " << e.what());
    return {};
  }
}

template <>
github_release_utils::GitHubRelease jsonToValue(const Json::Value& value) {
  return github_release_utils::GitHubRelease{
      .url = value["url"].asString(),
      .id = value["id"].asInt(),
      .tag_name = value["tag_name"].asString(),
      .name = value["name"].asString(),
      .draft = value["draft"].asBool(),
      .prerelease = value["prerelease"].asBool(),
      .created_at = value["created_at"].asString(),
      .published_at = value["published_at"].asString(),
      .assets =
          ParseJsonArray<github_release_utils::GitHubAsset>(value["assets"]),
  };
}
};  // namespace json_parser_utils
