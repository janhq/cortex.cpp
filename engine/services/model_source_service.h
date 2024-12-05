#pragma once
#include "utils/result.hpp"

// struct ModelEntry {
//   std::string model;
//   std::string author_repo_id;
//   std::string branch_name;
//   std::string path_to_model_yaml;
//   std::string model_alias;
//   std::string model_format;
//   std::string model_source;
//   ModelStatus status;
//   std::string engine;
// };
namespace services {
class ModelSourceService {
 public:
  // model source can be HF organization, repo or others (for example Modelscope,..)
  // default is HF, need to check if it is organization or repo
  // if repo:
  // if org: api/models?author=cortexso
  cpp::result<bool, std::string> AddModelSource(
      const std::string& model_source);

  cpp::result<bool, std::string> RemoveModelSource(
      const std::string& model_source);

 private:
  // models database
  cpp::result<bool, std::string> AddOrg(const std::string& org);
  cpp::result<bool, std::string> AddRepo(const std::string& model_source,
                                         const std::string& author,
                                         const std::string& model_name);

  cpp::result<bool, std::string> RemoveOrg(const std::string& org);
  cpp::result<bool, std::string> RemoveRepo(const std::string& repo);
};
}  // namespace services