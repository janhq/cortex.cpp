#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "utils/curl_utils.h"
#include "utils/engine_constants.h"
#include "utils/json_parser_utils.h"
#include "utils/result.hpp"
#include "utils/url_parser.h"

namespace huggingface_utils {

struct HuggingFaceBranch {
  std::string name;
  std::string ref;
  std::string targetCommit;
};

struct HuggingFaceFileSibling {
  std::string rfilename;
};

struct HuggingFaceGgufInfo {
  uint64_t total;
  std::string architecture;

  static cpp::result<HuggingFaceGgufInfo, std::string> FromJson(
      const Json::Value& json) {
    if (json.isNull() || json.type() == Json::ValueType::nullValue) {
      return cpp::fail("gguf info is null");
    }
    try {
      return HuggingFaceGgufInfo{
          .total = json["total"].asUInt64(),
          .architecture = json["architecture"].asString(),
      };
    } catch (const std::exception& e) {
      return cpp::fail("Failed to parse gguf info: " + std::string(e.what()));
    }
  }

  Json::Value ToJson() {
    Json::Value root;
    root["total"] = total;
    root["architecture"] = architecture;
    return root;
  }
};

struct HuggingFaceModelRepoInfo {
  std::string id;
  std::string modelId;
  std::string author;
  std::string sha;
  std::string lastModified;

  bool isPrivate;
  bool disabled;
  bool gated;
  std::vector<std::string> tags;
  int downloads;

  int likes;
  std::optional<HuggingFaceGgufInfo> gguf;
  std::vector<HuggingFaceFileSibling> siblings;
  std::vector<std::string> spaces;
  std::string createdAt;

  static cpp::result<HuggingFaceModelRepoInfo, std::string> FromJson(
      const Json::Value& body) {
    std::optional<HuggingFaceGgufInfo> gguf = std::nullopt;
    auto gguf_result = HuggingFaceGgufInfo::FromJson(body["gguf"]);
    if (gguf_result.has_value()) {
      gguf = gguf_result.value();
    }

    std::vector<HuggingFaceFileSibling> siblings{};
    auto siblings_info = body["siblings"];
    for (const auto& sibling : siblings_info) {
      auto sibling_info = HuggingFaceFileSibling{
          .rfilename = sibling["rfilename"].asString(),
      };
      siblings.push_back(sibling_info);
    }

    return HuggingFaceModelRepoInfo{
        .id = body["id"].asString(),
        .modelId = body["modelId"].asString(),
        .author = body["author"].asString(),
        .sha = body["sha"].asString(),
        .lastModified = body["lastModified"].asString(),

        .isPrivate = body["private"].asBool(),
        .disabled = body["disabled"].asBool(),
        .gated = body["gated"].asBool(),
        .tags = json_parser_utils::ParseJsonArray<std::string>(body["tags"]),
        .downloads = body["downloads"].asInt(),

        .likes = body["likes"].asInt(),
        .gguf = gguf,
        .siblings = siblings,
        .spaces =
            json_parser_utils::ParseJsonArray<std::string>(body["spaces"]),
        .createdAt = body["createdAt"].asString(),
    };
  }

  Json::Value ToJson() {
    Json::Value root;
    root["gguf"] = gguf->ToJson();
    return root;
  }
};

inline cpp::result<std::unordered_map<std::string, HuggingFaceBranch>,
                   std::string>
GetModelRepositoryBranches(const std::string& author,
                           const std::string& modelName) {
  if (author.empty() || modelName.empty()) {
    return cpp::fail("Author and model name cannot be empty");
  }
  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = kHuggingFaceHost,
      .pathParams = {"api", "models", author, modelName, "refs"}};

  auto result = curl_utils::SimpleGetJson(url_obj.ToFullPath());
  if (result.has_error()) {
    return cpp::fail("Failed to get model repository branches: " + author +
                     "/" + modelName);
  }

  auto branches_json = result.value()["branches"];
  std::unordered_map<std::string, HuggingFaceBranch> branches{};

  for (const auto& branch : branches_json) {
    branches[branch["name"].asString()] = HuggingFaceBranch{
        .name = branch["name"].asString(),
        .ref = branch["ref"].asString(),
        .targetCommit = branch["targetCommit"].asString(),
    };
  }

  return branches;
}

// only support gguf for now
inline cpp::result<HuggingFaceModelRepoInfo, std::string>
GetHuggingFaceModelRepoInfo(const std::string& author,
                            const std::string& modelName) {
  if (author.empty() || modelName.empty()) {
    return cpp::fail("Author and model name cannot be empty");
  }
  auto url_obj =
      url_parser::Url{.protocol = "https",
                      .host = kHuggingFaceHost,
                      .pathParams = {"api", "models", author, modelName}};

  auto result = curl_utils::SimpleGetJson(url_obj.ToFullPath());
  if (result.has_error()) {
    return cpp::fail("Failed to get model repository info: " + author + "/" +
                     modelName);
  }

  return HuggingFaceModelRepoInfo::FromJson(result.value());
}

inline std::string GetMetadataUrl(const std::string& model_id) {
  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = kHuggingFaceHost,
      .pathParams = {"cortexso", model_id, "resolve", "main", "metadata.yml"}};

  return url_obj.ToFullPath();
}

inline std::string GetDownloadableUrl(const std::string& author,
                                      const std::string& modelName,
                                      const std::string& fileName,
                                      const std::string& branch = "main") {
  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = kHuggingFaceHost,
      .pathParams = {author, modelName, "resolve", branch, fileName},
  };
  return url_parser::FromUrl(url_obj);
}

inline std::optional<std::string> GetDefaultBranch(
    const std::string& model_name) {
  try {
    auto default_model_branch =
        curl_utils::ReadRemoteYaml(GetMetadataUrl(model_name));

    if (default_model_branch.has_error()) {
      return std::nullopt;
    }

    auto metadata = default_model_branch.value();
    auto default_branch = metadata["default"];
    if (default_branch.IsDefined()) {
      return default_branch.as<std::string>();
    }
    return std::nullopt;
  } catch (const std::exception& e) {
    return std::nullopt;
  }
}
}  // namespace huggingface_utils
