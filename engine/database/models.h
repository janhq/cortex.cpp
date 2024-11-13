#pragma once

#include <SQLiteCpp/Database.h>
#include <trantor/utils/Logger.h>
#include <string>
#include <vector>
#include "utils/result.hpp"

namespace cortex::db {

enum class ModelStatus {
  Remote,
  Downloaded,
  Undownloaded
};

struct ModelEntry {
  std::string model;
  std::string model_format;
  std::string model_source;
  ModelStatus status;
  std::string engine;
  std::string author_repo_id;
  std::string branch_name;
  std::string path_to_model_yaml;
  std::string model_alias;
};

class Models {

 private:
  SQLite::Database& db_;

  bool IsUnique(const std::vector<ModelEntry>& entries,
                const std::string& model_id,
                const std::string& model_alias) const;

  cpp::result<std::vector<ModelEntry>, std::string> LoadModelListNoLock() const;

  std::string StatusToString(ModelStatus status) const;
  ModelStatus StringToStatus(const std::string& status_str) const;

 public:
  cpp::result<std::vector<ModelEntry>, std::string> LoadModelList() const;
  Models();
  Models(SQLite::Database& db);
  ~Models();
  std::string GenerateShortenedAlias(
      const std::string& model_id,
      const std::vector<ModelEntry>& entries) const;
  cpp::result<ModelEntry, std::string> GetModelInfo(
      const std::string& identifier) const;
  void PrintModelInfo(const ModelEntry& entry) const;
  cpp::result<bool, std::string> AddModelEntry(ModelEntry new_entry,
                                               bool use_short_alias = false);
  cpp::result<bool, std::string> UpdateModelEntry(
      const std::string& identifier, const ModelEntry& updated_entry);
  cpp::result<bool, std::string> DeleteModelEntry(
      const std::string& identifier);
  cpp::result<bool, std::string> UpdateModelAlias(
      const std::string& model_id, const std::string& model_alias);
  cpp::result<std::vector<std::string>, std::string> FindRelatedModel(
      const std::string& identifier) const;
  bool HasModel(const std::string& identifier) const;
};

}  // namespace cortex::db