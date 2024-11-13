#pragma once

#include <SQLiteCpp/Database.h>
#include <trantor/utils/Logger.h>
#include <string>
#include <vector>
#include <optional>

namespace cortex::db {

struct EngineEntry {
    std::string engine;
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

  std::optional<std::string> UpsertEngine(const std::string& engine_name,
                                          const std::string& type,
                                          const std::string& api_key,
                                          const std::string& url,
                                          const std::string& version,
                                          const std::string& variant,
                                          const std::string& status,
                                          const std::string& metadata);

  std::optional<EngineEntry> GetEngine(int id, const std::string& engine_name) const;

  std::optional<std::string> DeleteEngine(int id);
};

}  // namespace cortex::db