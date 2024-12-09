#pragma once
#include <atomic>
#include <thread>
#include <unordered_set>
#include "utils/result.hpp"

namespace services {
class ModelSourceService {
 public:
  explicit ModelSourceService();
  ~ModelSourceService();
  // model source can be HF organization, repo or others (for example Modelscope,..)
  // default is HF, need to check if it is organization or repo
  // if repo:
  // if org: api/models?author=cortexso
  cpp::result<bool, std::string> AddModelSource(
      const std::string& model_source);

  cpp::result<bool, std::string> RemoveModelSource(
      const std::string& model_source);

  cpp::result<std::vector<std::string>, std::string> GetModelSources();

 private:
  cpp::result<bool, std::string> AddHfOrg(const std::string& model_source,
                                          const std::string& author);

  cpp::result<bool, std::string> AddHfRepo(
      const std::string& model_source, const std::string& author,
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
                        const std::string& branch);

  void SyncModelSource();

 private:
  std::thread sync_db_thread_;
  std::atomic<bool> running_;
};
}  // namespace services