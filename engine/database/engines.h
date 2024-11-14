#pragma once

#include <SQLiteCpp/Database.h>
#include <trantor/utils/Logger.h>
#include <string>
#include <vector>
#include <optional>

namespace cortex::db {

struct EngineEntry {
    int id;
    std::string engine_name;
    std::string type;
    std::string api_key;
    std::string url;
    std::string version;
    std::string variant;
    std::string status;
    std::string metadata;
    std::string date_created;
    std::string date_updated;
};

class Engines {
 private:
  SQLite::Database& db_;

  bool IsUnique(const std::vector<EngineEntry>& entries,
                const std::string& model_id,
                const std::string& model_alias) const;

  std::optional<std::vector<EngineEntry>> LoadModelListNoLock() const;

 public:
  Engines();
  Engines(SQLite::Database& db);
  ~Engines();

  std::optional<EngineEntry> UpsertEngine(const std::string& engine_name,
                                          const std::string& type,
                                          const std::string& api_key,
                                          const std::string& url,
                                          const std::string& version,
                                          const std::string& variant,
                                          const std::string& status,
                                          const std::string& metadata);

  std::optional<std::vector<EngineEntry>> GetEngines() const;
  std::optional<EngineEntry> GetEngineById(int id) const;
  std::optional<EngineEntry> GetEngineByNameAndVariant(const std::string& engine_name, const std::optional<std::string> variant = std::nullopt) const;

  std::optional<std::string> DeleteEngineById(int id);
};

}  // namespace cortex::db