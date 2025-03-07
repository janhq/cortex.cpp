#pragma once
#include <atomic>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "services/database_service.h"
#include "utils/result.hpp"

struct ModelSourceInfo {
  std::string id;
  uint64_t size;
  Json::Value ToJson() const {
    Json::Value root;
    root["id"] = id;
    root["size"] = size;
    return root;
  }
};

struct ModelSource {
  std::string id;
  std::string author;
  std::vector<ModelSourceInfo> models;
  Json::Value metadata;

  Json::Value ToJson() {
    Json::Value root;
    root["id"] = id;
    root["author"] = author;
    Json::Value models_json;
    for (auto const& m : models) {
      models_json.append(m.ToJson());
    }
    root["models"] = models_json;
    root["metadata"] = metadata;
    return root;
  };
};

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

class ModelSourceService {
 public:
  explicit ModelSourceService(std::shared_ptr<DatabaseService> db_service);
  ~ModelSourceService();

  cpp::result<bool, std::string> AddModelSource(
      const std::string& model_source);

  cpp::result<bool, std::string> RemoveModelSource(
      const std::string& model_source);

  cpp::result<std::unordered_map<std::string, ModelSource>, std::string>
  GetModelSources();

  cpp::result<ModelSource, std::string> GetModelSource(const std::string& src);

  cpp::result<std::vector<std::string>, std::string> GetRepositoryList(
      std::string_view hub_author, std::string_view tag_filter);

 private:
  cpp::result<bool, std::string> AddHfOrg(const std::string& model_source,
                                          const std::string& hub_author);

  cpp::result<bool, std::string> AddHfRepo(const std::string& model_source,
                                           const std::string& hub_author,
                                           const std::string& model_name);

  cpp::result<std::unordered_set<std::string>, std::string> AddRepoSiblings(
      const std::string& model_source, const std::string& hub_author,
      const std::string& model_name);

  cpp::result<bool, std::string> AddCortexsoOrg(
      const std::string& model_source);

  cpp::result<bool, std::string> AddCortexsoRepo(
      const std::string& model_source, const std::string& hub_author,
      const std::string& model_name);

  cpp::result<std::string, std::string> AddCortexsoRepoBranch(
      const std::string& model_source, const std::string& author,
      const std::string& model_name, const std::string& branch,
      const std::string& metadata, const std::string& desc);

  void SyncModelSource();

 private:
  std::shared_ptr<DatabaseService> db_service_ = nullptr;
  std::thread sync_db_thread_;
  std::atomic<bool> running_;

  std::unordered_map<std::string, std::vector<ModelInfo>> cortexso_repos_;
  using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
  std::unordered_map<std::string, TimePoint> src_cache_;
};