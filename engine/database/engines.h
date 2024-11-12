#pragma once

#include <SQLiteCpp/Database.h>
#include <trantor/utils/Logger.h>
#include <string>
#include <vector>
#include "utils/result.hpp"

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

  cpp::result<std::vector<EngineEntry>, std::string> LoadModelListNoLock() const;

 public:
  Engines();
  Engines(SQLite::Database& db);
  ~Engines();
};

}  // namespace cortex::db