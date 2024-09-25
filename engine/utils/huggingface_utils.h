#pragma once

#include <httplib.h>
#include <optional>
#include <string>
#include <vector>
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

inline cpp::result<std::vector<HuggingFaceBranch>, std::string>
GetModelRepositoryBranches(const std::string& author,
                           const std::string& modelName) {
  if (author.empty() || modelName.empty()) {
    return cpp::fail("Author and model name cannot be empty");
  }
  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = kHuggingfaceHost,
      .pathParams = {"api", "models", author, modelName, "refs"}};

  httplib::Client cli(url_obj.GetProtocolAndHost());
  auto res = cli.Get(url_obj.GetPathAndQuery());
  if (res->status != httplib::StatusCode::OK_200) {
    return cpp::fail("Failed to get model repository branches: " + author +
                     "/" + modelName);
  }

  using json = nlohmann::json;
  auto body = json::parse(res->body);
  auto branches_json = body["branches"];

  std::vector<HuggingFaceBranch> branches{};

  for (const auto& branch : branches_json) {
    branches.push_back(HuggingFaceBranch{
        .name = branch["name"],
        .ref = branch["ref"],
        .targetCommit = branch["targetCommit"],
    });
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

  httplib::Client cli(url_obj.GetProtocolAndHost());
  auto res = cli.Get(url_obj.GetPathAndQuery());
  if (res->status != httplib::StatusCode::OK_200) {
    return cpp::fail("Failed to get model repository info: " + author + "/" +
                     modelName);
  }

  using json = nlohmann::json;
  auto body = json::parse(res->body);

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
}  // namespace huggingface_utils
