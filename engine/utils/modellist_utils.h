#pragma once
#include <mutex>
#include <string>
#include <vector>
#include "logging_utils.h"
#include <trantor/utils/Logger.h>
namespace modellist_utils {

enum class ModelStatus { READY, RUNNING };

struct ModelEntry {
  std::string model_id;
  std::string author_repo_id;
  std::string branch_name;
  std::string path_to_model_yaml;
  std::string model_alias;
  ModelStatus status;
};

class ModelListUtils {
 private:
  mutable std::mutex mutex_;  // For thread safety
  static const std::string kModelListPath;

  std::vector<ModelEntry> LoadModelList() const;
  bool IsUnique(const std::vector<ModelEntry>& entries,
                const std::string& model_id,
                const std::string& model_alias) const;
  void SaveModelList(const std::vector<ModelEntry>& entries) const;
  std::string GenerateShortenedAlias(
      const std::string& model_id,
      const std::vector<ModelEntry>& entries) const;

 public:
  ModelListUtils() = default;

  ModelEntry GetModelInfo(const std::string& identifier) const;
  void PrintModelInfo(const ModelEntry& entry) const;
  bool AddModelEntry(ModelEntry new_entry, bool use_short_alias = true);
  bool UpdateModelEntry(const std::string& identifier,
                        const ModelEntry& updated_entry);
  bool DeleteModelEntry(const std::string& identifier);
};
}  // namespace modellist_utils