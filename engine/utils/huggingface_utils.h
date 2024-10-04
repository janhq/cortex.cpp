#pragma once

#include <optional>
#include <string>
#include <vector>
#include "utils/curl_utils.h"
#include "utils/json.hpp"
#include "utils/result.hpp"
#include "utils/url_parser.h"

namespace huggingface_utils {

constexpr static auto kHuggingfaceHost{"huggingface.co"};

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
      .host = kHuggingfaceHost,
      .pathParams = {"api", "models", author, modelName, "refs"}};

  auto result = curl_utils::SimpleGetJson(url_obj.ToFullPath());
  if (result.has_error()) {
    return cpp::fail("Failed to get model repository branches: " + author +
                     "/" + modelName);
  }

  auto branches_json = result.value()["branches"];
  std::unordered_map<std::string, HuggingFaceBranch> branches{};

  for (const auto& branch : branches_json) {
    branches[branch["name"]] = HuggingFaceBranch{
        .name = branch["name"],
        .ref = branch["ref"],
        .targetCommit = branch["targetCommit"],
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
                      .host = kHuggingfaceHost,
                      .pathParams = {"api", "models", author, modelName}};

  auto result = curl_utils::SimpleGetJson(url_obj.ToFullPath());
  if (result.has_error()) {
    return cpp::fail("Failed to get model repository info: " + author + "/" +
                     modelName);
  }

  auto body = result.value();

  std::optional<HuggingFaceGgufInfo> gguf = std::nullopt;
  auto gguf_info = body["gguf"];
  if (!gguf_info.is_null()) {
    gguf = HuggingFaceGgufInfo{
        .total = gguf_info["total"],
        .architecture = gguf_info["architecture"],
    };
  }

  std::vector<HuggingFaceFileSibling> siblings{};
  auto siblings_info = body["siblings"];
  for (const auto& sibling : siblings_info) {
    auto sibling_info = HuggingFaceFileSibling{
        .rfilename = sibling["rfilename"],
    };
    siblings.push_back(sibling_info);
  }

  auto model_repo_info = HuggingFaceModelRepoInfo{
      .id = body["id"],
      .modelId = body["modelId"],
      .author = body["author"],
      .sha = body["sha"],
      .lastModified = body["lastModified"],

      .isPrivate = body["private"],
      .disabled = body["disabled"],
      .gated = body["gated"],
      .tags = body["tags"],
      .downloads = body["downloads"],

      .likes = body["likes"],
      .gguf = gguf,
      .siblings = siblings,
      .spaces = body["spaces"],
      .createdAt = body["createdAt"],
  };

  return model_repo_info;
}

inline std::string GetMetadataUrl(const std::string& model_id) {
  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = kHuggingfaceHost,
      .pathParams = {"cortexso", model_id, "resolve", "main", "metadata.yml"}};

  return url_obj.ToFullPath();
}

inline std::string GetDownloadableUrl(const std::string& author,
                                      const std::string& modelName,
                                      const std::string& fileName,
                                      const std::string& branch = "main") {
  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = kHuggingfaceHost,
      .pathParams = {author, modelName, "resolve", branch, fileName},
  };
  return url_parser::FromUrl(url_obj);
}

inline std::optional<std::string> GetDefaultBranch(
    const std::string& model_name) {
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
}
}  // namespace huggingface_utils
