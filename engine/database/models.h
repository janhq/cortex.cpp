#pragma once

#include <trantor/utils/Logger.h>
#include <string>
#include <vector>
#include "SQLiteCpp/SQLiteCpp.h"
#include "utils/result.hpp"

namespace cortex::db {

enum class ModelStatus { READY, RUNNING };

struct ModelEntry {
  std::string model_id;
  std::string author_repo_id;
  std::string branch_name;
  std::string path_to_model_yaml;
  std::string model_alias;
  ModelStatus status;
};

class Models {

 private:
  SQLite::Database& db_;

  bool IsUnique(const std::string& model_id,
                const std::string& model_alias) const;

 public:
  static const std::string kModelListPath;
  cpp::result<std::vector<ModelEntry>, std::string> LoadModelList() const;
  Models();
  Models(SQLite::Database& db);
  ~Models();
  std::string GenerateShortenedAlias(const std::string& model_id) const;
  ModelEntry GetModelInfo(const std::string& identifier) const;
  void PrintModelInfo(const ModelEntry& entry) const;
  bool AddModelEntry(ModelEntry new_entry, bool use_short_alias = false);
  bool UpdateModelEntry(const std::string& identifier,
                        const ModelEntry& updated_entry);
  bool DeleteModelEntry(const std::string& identifier);
  bool UpdateModelAlias(const std::string& model_id,
                        const std::string& model_alias);
  bool HasModel(const std::string& identifier) const;
};
}  // namespace cortex::db
