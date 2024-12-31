#pragma once
#include <atomic>
#include <thread>
#include <unordered_set>
#include "services/database_service.h"
#include "utils/result.hpp"

class ModelSourceService {
 public:
  explicit ModelSourceService(std::shared_ptr<DatabaseService> db_service);
  ~ModelSourceService();

  cpp::result<bool, std::string> AddModelSource(
      const std::string& model_source);

  cpp::result<bool, std::string> RemoveModelSource(
      const std::string& model_source);

  cpp::result<std::vector<std::string>, std::string> GetModelSources();

 private:
  cpp::result<bool, std::string> AddHfOrg(const std::string& model_source,
                                          const std::string& author);

  cpp::result<bool, std::string> AddHfRepo(const std::string& model_source,
                                           const std::string& author,
                                           const std::string& model_name);

  cpp::result<std::unordered_set<std::string>, std::string> AddRepoSiblings(
      const std::string& model_source, const std::string& author,
      const std::string& model_name);

  cpp::result<bool, std::string> AddCortexsoOrg(
      const std::string& model_source);

  cpp::result<bool, std::string> AddCortexsoRepo(
      const std::string& model_source, const std::string& author,
      const std::string& model_name);

  cpp::result<std::unordered_set<std::string>, std::string>
  AddCortexsoRepoBranch(const std::string& model_source,
                        const std::string& author,
                        const std::string& model_name,
                        const std::string& branch, const std::string& metadata);

  void SyncModelSource();

 private:
  std::shared_ptr<DatabaseService> db_service_ = nullptr;
  std::thread sync_db_thread_;
  std::atomic<bool> running_;
};