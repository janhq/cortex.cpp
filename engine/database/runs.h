#pragma once

#include <SQLiteCpp/Database.h>
#include <trantor/utils/Logger.h>
#include <string>
#include <vector>
#include "common/run.h"
#include "database.h"
#include "utils/result.hpp"

namespace cortex::db {
class Runs {
  SQLite::Database& db_;

 public:
  Runs(SQLite::Database& db) : db_{db} {};

  Runs() : db_(cortex::db::Database::GetInstance().db()) {}

  ~Runs() {}

  cpp::result<std::vector<OpenAi::Run>, std::string> ListRuns(
      uint8_t limit, const std::string& order, const std::string& after,
      const std::string& before) const;

  cpp::result<OpenAi::Run, std::string> RetrieveRun(
      const std::string& run_id) const;

  cpp::result<void, std::string> UpdateRun(const OpenAi::Run& run);

  cpp::result<void, std::string> AddRunEntry(const OpenAi::Run& run);

  cpp::result<void, std::string> DeleteRun(const std::string& run_id);

 private:
  OpenAi::Run ParseRunFromQuery(SQLite::Statement& query) const;
};
}  // namespace cortex::db
