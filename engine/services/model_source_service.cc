#include "model_source_service.h"
#include <chrono>
#include <unordered_set>
#include "database/models.h"
#include "json/json.h"
#include "utils/curl_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"
#include "utils/url_parser.h"

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

ModelSourceService::ModelSourceService(
    std::shared_ptr<DatabaseService> db_service)
    : db_service_(db_service) {
  sync_db_thread_ = std::thread(&ModelSourceService::SyncModelSource, this);
  running_ = true;
}

ModelSourceService::~ModelSourceService() {
  running_ = false;
  if (sync_db_thread_.joinable()) {
    sync_db_thread_.join();
  }
  CTL_INF("Done cleanup thread");
}

cpp::result<bool, std::string> ModelSourceService::AddModelSource(
    const std::string& model_source) {
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
        return AddCortexsoOrg(model_source);
      } else {
        return AddHfOrg(model_source, author);
      }
    } else {  // Repo
      auto const& author = r.pathParams[0];
      auto const& model_name = r.pathParams[1];
      if (r.pathParams[0] == "cortexso") {
        return AddCortexsoRepo(model_source, author, model_name);
      } else {
        return AddHfRepo(model_source, author, model_name);
      }
    }
  }
  return true;
}

cpp::result<bool, std::string> ModelSourceService::RemoveModelSource(
    const std::string& model_source) {
  auto srcs = db_service_->GetModelSources();
  if (srcs.has_error()) {
    return cpp::fail(srcs.error());
  } else {
    auto& v = srcs.value();
    if (std::find(v.begin(), v.end(), model_source) == v.end()) {
      return cpp::fail("Model source does not exist: " + model_source);
    }
  }
  CTL_INF("Remove model source: " << model_source);
  auto res = url_parser::FromUrlString(model_source);
  if (res.has_error()) {
    return cpp::fail(res.error());
  } else {
    auto& r = res.value();
    if (r.pathParams.empty() || r.pathParams.size() > 2) {
      return cpp::fail("Invalid model source url: " + model_source);
    }

    if (r.pathParams.size() == 1) {
      if (auto del_res = db_service_->DeleteModelEntryWithOrg(model_source);
          del_res.has_error()) {
        CTL_INF(del_res.error());
        return cpp::fail(del_res.error());
      }
    } else {
      if (auto del_res = db_service_->DeleteModelEntryWithRepo(model_source);
          del_res.has_error()) {
        CTL_INF(del_res.error());
        return cpp::fail(del_res.error());
      }
    }
  }
  return true;
}

cpp::result<std::vector<std::string>, std::string>
ModelSourceService::GetModelSources() {
  return db_service_->GetModelSources();
}

cpp::result<bool, std::string> ModelSourceService::AddHfOrg(
    const std::string& model_source, const std::string& author) {
  auto res = curl_utils::SimpleGet("https://huggingface.co/api/models?author=" +
                                   author);
  if (res.has_value()) {
    auto models = ParseJsonString(res.value());
    // Get models from db

    auto model_list_before = db_service_->GetModels(model_source)
                                 .value_or(std::vector<std::string>{});
    std::unordered_set<std::string> updated_model_list;
    // Add new models
    for (auto const& m : models) {
      CTL_DBG(m.id);
      auto author_model = string_utils::SplitBy(m.id, "/");
      if (author_model.size() == 2) {
        auto const& author = author_model[0];
        auto const& model_name = author_model[1];
        auto add_res = AddRepoSiblings(model_source, author, model_name)
                           .value_or(std::unordered_set<std::string>{});
        for (auto const& a : add_res) {
          updated_model_list.insert(a);
        }
      }
    }

    // Clean up
    for (auto const& mid : model_list_before) {
      if (updated_model_list.find(mid) == updated_model_list.end()) {
        if (auto del_res = db_service_->DeleteModelEntry(mid);
            del_res.has_error()) {
          CTL_INF(del_res.error());
        }
      }
    }
  } else {
    return cpp::fail(res.error());
  }
  return true;
}

cpp::result<bool, std::string> ModelSourceService::AddHfRepo(
    const std::string& model_source, const std::string& author,
    const std::string& model_name) {
  // Get models from db

  auto model_list_before =
      db_service_->GetModels(model_source).value_or(std::vector<std::string>{});
  std::unordered_set<std::string> updated_model_list;
  auto add_res = AddRepoSiblings(model_source, author, model_name);
  if (add_res.has_error()) {
    return cpp::fail(add_res.error());
  } else {
    updated_model_list = add_res.value();
  }
  for (auto const& mid : model_list_before) {
    if (updated_model_list.find(mid) == updated_model_list.end()) {
      if (auto del_res = db_service_->DeleteModelEntry(mid);
          del_res.has_error()) {
        CTL_INF(del_res.error());
      }
    }
  }
  return true;
}

cpp::result<std::unordered_set<std::string>, std::string>
ModelSourceService::AddRepoSiblings(const std::string& model_source,
                                    const std::string& author,
                                    const std::string& model_name) {
  std::unordered_set<std::string> res;
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
      std::string model_id =
          author + ":" + model_name + ":" + sibling.rfilename;
      cortex::db::ModelEntry e = {
          .model = model_id,
          .author_repo_id = author,
          .branch_name = "main",
          .path_to_model_yaml = "",
          .model_alias = "",
          .model_format = "hf-gguf",
          .model_source = model_source,
          .status = cortex::db::ModelStatus::Downloadable,
          .engine = "llama-cpp",
          .metadata = repo_info->metadata};
      if (!db_service_->HasModel(model_id)) {
        if (auto add_res = db_service_->AddModelEntry(e); add_res.has_error()) {
          CTL_INF(add_res.error());
        }
      } else {
        if (auto m = db_service_->GetModelInfo(model_id);
            m.has_value() &&
            m->status == cortex::db::ModelStatus::Downloadable) {
          if (auto upd_res = db_service_->UpdateModelEntry(model_id, e);
              upd_res.has_error()) {
            CTL_INF(upd_res.error());
          }
        }
      }
      res.insert(model_id);
    }
  }

  return res;
}

cpp::result<bool, std::string> ModelSourceService::AddCortexsoOrg(
    const std::string& model_source) {
  auto res = curl_utils::SimpleGet(
      "https://huggingface.co/api/models?author=cortexso");
  if (res.has_value()) {
    auto models = ParseJsonString(res.value());
    // Get models from db

    auto model_list_before = db_service_->GetModels(model_source)
                                 .value_or(std::vector<std::string>{});
    std::unordered_set<std::string> updated_model_list;
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

        auto repo_info = hu::GetHuggingFaceModelRepoInfo(author, model_name);
        if (repo_info.has_error()) {
          CTL_INF(repo_info.error());
          continue;
        }
        for (auto const& [branch, _] : branches.value()) {
          CTL_INF(branch);
          auto add_res = AddCortexsoRepoBranch(model_source, author, model_name,
                                               branch, repo_info->metadata)
                             .value_or(std::unordered_set<std::string>{});
          for (auto const& a : add_res) {
            updated_model_list.insert(a);
          }
        }
      }
    }
    // Clean up
    for (auto const& mid : model_list_before) {
      if (updated_model_list.find(mid) == updated_model_list.end()) {
        if (auto del_res = db_service_->DeleteModelEntry(mid);
            del_res.has_error()) {
          CTL_INF(del_res.error());
        }
      }
    }
  } else {
    return cpp::fail(res.error());
  }

  return true;
}

cpp::result<bool, std::string> ModelSourceService::AddCortexsoRepo(
    const std::string& model_source, const std::string& author,
    const std::string& model_name) {
  auto branches =
      huggingface_utils::GetModelRepositoryBranches("cortexso", model_name);
  if (branches.has_error()) {
    return cpp::fail(branches.error());
  }

  auto repo_info = hu::GetHuggingFaceModelRepoInfo(author, model_name);
  if (repo_info.has_error()) {
    return cpp::fail(repo_info.error());
  }
  // Get models from db

  auto model_list_before =
      db_service_->GetModels(model_source).value_or(std::vector<std::string>{});
  std::unordered_set<std::string> updated_model_list;

  for (auto const& [branch, _] : branches.value()) {
    CTL_INF(branch);
    auto add_res = AddCortexsoRepoBranch(model_source, author, model_name,
                                         branch, repo_info->metadata)
                       .value_or(std::unordered_set<std::string>{});
    for (auto const& a : add_res) {
      updated_model_list.insert(a);
    }
  }

  // Clean up
  for (auto const& mid : model_list_before) {
    if (updated_model_list.find(mid) == updated_model_list.end()) {
      if (auto del_res = db_service_->DeleteModelEntry(mid);
          del_res.has_error()) {
        CTL_INF(del_res.error());
      }
    }
  }
  return true;
}

cpp::result<std::unordered_set<std::string>, std::string>
ModelSourceService::AddCortexsoRepoBranch(const std::string& model_source,
                                          const std::string& author,
                                          const std::string& model_name,
                                          const std::string& branch,
                                          const std::string& metadata) {
  std::unordered_set<std::string> res;

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
    return {};
  } else {
    std::string model_id = model_name + ":" + branch;
    cortex::db::ModelEntry e = {.model = model_id,
                                .author_repo_id = author,
                                .branch_name = branch,
                                .path_to_model_yaml = "",
                                .model_alias = "",
                                .model_format = "cortexso",
                                .model_source = model_source,
                                .status = cortex::db::ModelStatus::Downloadable,
                                .engine = "llama-cpp",
                                .metadata = metadata};
    if (!db_service_->HasModel(model_id)) {
      CTL_INF("Adding model to db: " << model_name << ":" << branch);
      if (auto res = db_service_->AddModelEntry(e);
          res.has_error() || !res.value()) {
        CTL_DBG("Cannot add model to db: " << model_id);
      }
    } else {
      if (auto m = db_service_->GetModelInfo(model_id);
          m.has_value() && m->status == cortex::db::ModelStatus::Downloadable) {
        if (auto upd_res = db_service_->UpdateModelEntry(model_id, e);
            upd_res.has_error()) {
          CTL_INF(upd_res.error());
        }
      }
    }
    res.insert(model_id);
  }
  return res;
}

void ModelSourceService::SyncModelSource() {
  // Do interval check for 10 minutes
  constexpr const int kIntervalCheck = 10 * 60;
  auto start_time = std::chrono::steady_clock::now();
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
                            current_time - start_time)
                            .count();

    if (elapsed_time > kIntervalCheck) {
      CTL_DBG("Start to sync cortex.db");
      start_time = current_time;

      auto res = db_service_->GetModelSources();
      if (res.has_error()) {
        CTL_INF(res.error());
      } else {
        for (auto const& src : res.value()) {
          CTL_DBG(src);
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
        for (auto const& c : update_cand) {
          if (auto res = AddModelSource(c); res.has_error()) {
            CTL_INF(res.error();)
          }
        }
      }

      CTL_DBG("Done sync cortex.db");
    }
  }
}
