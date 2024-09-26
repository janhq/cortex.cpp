#pragma once

#include <trantor/utils/Logger.h>
#include <mutex>
#include <string>
#include <vector>

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

  bool IsUnique(const std::vector<ModelEntry>& entries,
                const std::string& model_id,
                const std::string& model_alias) const;
  void SaveModelList(const std::vector<ModelEntry>& entries) const;

 public:
  static const std::string kModelListPath;
  std::vector<ModelEntry> LoadModelList() const;
  ModelListUtils() = default;
  std::string GenerateShortenedAlias(
      const std::string& model_id,
      const std::vector<ModelEntry>& entries) const;
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
}  // namespace modellist_utils
