#include "model_source_service.h"
#include <unordered_set>
#include "database/models.h"
#include "utils/curl_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"
#include "utils/url_parser.h"

namespace services {
namespace hu = huggingface_utils;

namespace {
struct ModelInfo {
  std::string id;
  int likes;
  int trending_score;
  bool is_private;
  int downloads;
  std::vector<std::string> tags;
  std::string created_at;
  std::string model_id;
};

std::vector<ModelInfo> ParseJsonString(const std::string& json_str) {
  std::vector<ModelInfo> models;

  // Parse the JSON string
  Json::Value root;
  Json::Reader reader;
  bool parsing_successful = reader.parse(json_str, root);

  if (!parsing_successful) {
    std::cerr << "Failed to parse JSON" << std::endl;
    return models;
  }

  // Iterate over the JSON array
  for (const auto& model : root) {
    ModelInfo info;
    info.id = model["id"].asString();
    info.likes = model["likes"].asInt();
    info.trending_score = model["trendingScore"].asInt();
    info.is_private = model["private"].asBool();
    info.downloads = model["downloads"].asInt();

    const Json::Value& tags = model["tags"];
    for (const auto& tag : tags) {
      info.tags.push_back(tag.asString());
    }

    info.created_at = model["createdAt"].asString();
    info.model_id = model["modelId"].asString();
    models.push_back(info);
  }

  return models;
}

}  // namespace
cpp::result<bool, std::string> ModelSourceService::AddModelSource(
    const std::string& model_source) {
  // https://huggingface.co/Orenguteng
  // https://huggingface.co/Orenguteng/Llama-3.1-8B-Lexi-Uncensored-V2-GGUF
  auto res = url_parser::FromUrlString(model_source);
  if (res.has_error()) {
    return cpp::fail(res.error());
  } else {
    auto& r = res.value();
    if (r.pathParams.empty() || r.pathParams.size() > 2) {
      return cpp::fail("Invalid model source url: " + model_source);
    }

    if (auto is_org = r.pathParams.size() == 1; is_org) {
      auto& author = r.pathParams[0];
      if (author == "cortexso") {
        if (auto res = curl_utils::SimpleGet(
                "https://huggingface.co/api/models?author=" + author);
            res.has_value()) {
          auto models = ParseJsonString(res.value());
          for (auto const& m : models) {
            CTL_INF(m.id);
            auto author_model = string_utils::SplitBy(m.id, "/");
            if (author_model.size() == 2) {
              auto const& author = author_model[0];
              auto const& model_name = author_model[1];
              auto branches = huggingface_utils::GetModelRepositoryBranches(
                  "cortexso", model_name);
              if (branches.has_error()) {
                CTL_INF(branches.error());
                continue;
              }
              for (auto const& [branch, _] : branches.value()) {
                CTL_INF(branch);
                AddCortexsoRepoBranch(model_source, author, model_name, branch);
              }
            }
          }
        }
      } else {
        if (auto res = curl_utils::SimpleGet(
                "https://huggingface.co/api/models?author=" + author);
            res.has_value()) {
          auto models = ParseJsonString(res.value());
          for (auto const& m : models) {
            CTL_INF(m.id);
            auto author_model = string_utils::SplitBy(m.id, "/");
            if (author_model.size() == 2) {
              auto const& author = author_model[0];
              auto const& model_name = author_model[1];
              AddRepo(model_source, author, model_name);
            }
          }
        }
      }

    } else {  // Repo
      auto const& author = r.pathParams[0];
      auto const& model_name = r.pathParams[1];
      if (r.pathParams[0] == "cortexso") {
        auto branches = huggingface_utils::GetModelRepositoryBranches(
            "cortexso", model_name);
        if (branches.has_error()) {
          return cpp::fail(branches.error());
        }
        for (auto const& [branch, _] : branches.value()) {
          CTL_INF(branch);
          AddCortexsoRepoBranch(model_source, author, model_name, branch);
        }
      } else {
        if (auto res = AddRepo(model_source, author, model_name);
            res.has_error()) {
          return cpp::fail(res.error());
        }
      }
    }
  }
  return true;
}

cpp::result<bool, std::string> ModelSourceService::RemoveModelSource(
    const std::string& model_source) {
  CTL_INF("Remove model source: " << model_source);
  auto res = url_parser::FromUrlString(model_source);
  if (res.has_error()) {
    return cpp::fail(res.error());
  } else {
    auto& r = res.value();
    if (r.pathParams.empty() || r.pathParams.size() > 2) {
      return cpp::fail("Invalid model source url: " + model_source);
    }
    cortex::db::Models model_db;
    if (r.pathParams.size() == 1) {
      if (auto del_res = model_db.DeleteModelEntryWithOrg(model_source);
          del_res.has_error()) {
        CTL_INF(del_res.error());
        return cpp::fail(del_res.error());
      }
    } else {
      if (auto del_res = model_db.DeleteModelEntryWithRepo(model_source);
          del_res.has_error()) {
        CTL_INF(del_res.error());
        return cpp::fail(del_res.error());
      }
    }
  }
  return true;
}

cpp::result<bool, std::string> ModelSourceService::AddOrg(
    const std::string& org) {
  return true;
}

cpp::result<bool, std::string> ModelSourceService::AddRepo(
    const std::string& model_source, const std::string& author,
    const std::string& model_name) {
  auto repo_info = hu::GetHuggingFaceModelRepoInfo(author, model_name);
  if (repo_info.has_error()) {
    return cpp::fail(repo_info.error());
  }

  if (!repo_info->gguf.has_value()) {
    return cpp::fail(
        "Not a GGUF model. Currently, only GGUF single file is "
        "supported.");
  }


  for (const auto& sibling : repo_info->siblings) {
    if (string_utils::EndsWith(sibling.rfilename, ".gguf")) {
      cortex::db::Models model_db;
      std::string model_id =
          author + ":" + model_name + ":" + sibling.rfilename;
      if (!model_db.HasModel(model_id)) {
        cortex::db::ModelEntry e = {
            .model = model_id,
            .author_repo_id = author,
            .branch_name = "main",
            .path_to_model_yaml = "",
            .model_alias = "",
            .model_format = "hf-gguf",
            .model_source = model_source,
            .status = cortex::db::ModelStatus::Undownloaded,
            .engine = "llama-cpp"};
        model_db.AddModelEntry(e);
      } else {
        // update
      }
    }
  }
  // delete old data
  return true;
}

cpp::result<bool, std::string> ModelSourceService::AddCortexsoRepoBranch(
    const std::string& model_source, const std::string& author,
    const std::string& model_name, const std::string& branch) {

  url_parser::Url url = {
      .protocol = "https",
      .host = kHuggingFaceHost,
      .pathParams = {"api", "models", "cortexso", model_name, "tree", branch},
  };

  auto result = curl_utils::SimpleGetJson(url.ToFullPath());
  if (result.has_error()) {
    return cpp::fail("Model " + model_name + " not found");
  }

  bool has_gguf = false;
  for (const auto& value : result.value()) {
    auto path = value["path"].asString();
    if (path.find(".gguf") != std::string::npos) {
      has_gguf = true;
    }
  }
  if (!has_gguf) {
    CTL_INF("Only support gguf file format! - branch: " << branch);
    return false;
  } else {
    cortex::db::Models model_db;
    std::string model_id = model_name + ":" + branch;
    if (!model_db.HasModel(model_id)) {
      CTL_INF("Adding model to db: " << model_name << ":" << branch);
      cortex::db::ModelEntry e = {
          .model = model_id,
          .author_repo_id = author,
          .branch_name = branch,
          .path_to_model_yaml = "",
          .model_alias = "",
          .model_format = "cortexso",
          .model_source = model_source,
          .status = cortex::db::ModelStatus::Undownloaded,
          .engine = "llama-cpp"};
      if (auto res = model_db.AddModelEntry(e);
          res.has_error() || !res.value()) {
        CTL_DBG("Cannot add model to db: " << model_id);
      }
    } else {
      CTL_DBG("Model exists: " << model_id);
    }
  }
  return true;
}

cpp::result<bool, std::string> ModelSourceService::RemoveOrg(
    const std::string& org) {
  return true;
}
cpp::result<bool, std::string> ModelSourceService::RemoveRepo(
    const std::string& repo) {
  return true;
}

void ModelSourceService::SyncModelSource() {
  cortex::db::Models model_db;
  auto res = model_db.GetModelSources();
  if (res.has_error()) {
    CTL_INF(res.error());
  } else {
    for (auto const& src : res.value()) {
      CTL_INF(src);
    }

    std::unordered_set<std::string> orgs;
    std::vector<std::string> repos;
    for (auto const& src : res.value()) {
      auto url_res = url_parser::FromUrlString(src);
      if (url_res.has_value()) {
        if (url_res->pathParams.size() == 1) {
          orgs.insert(src);
        } else if (url_res->pathParams.size() == 2) {
          repos.push_back(src);
        }
      }
    }

    // Get list to update
    std::vector<std::string> update_cand(orgs.begin(), orgs.end());
    auto get_org = [](const std::string& rp) {
      return rp.substr(0, rp.find_last_of("/"));
    };
    for (auto const& repo : repos) {
      if (orgs.find(get_org(repo)) != orgs.end()) {
        update_cand.push_back(repo);
      }
    }

    // Sync cortex.db with the upstream data
    // add new model if it does not exist
    // update model if it does exist
    // delete model if remote has removed model
    for (auto const& c : update_cand) {
      AddModelSource(c);
    }
  }
}

}  // namespace services