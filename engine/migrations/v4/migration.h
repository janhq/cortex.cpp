#pragma once

#include <SQLiteCpp/Database.h>
#include <string>
#include "utils/logging_utils.h"
#include "utils/result.hpp"

namespace cortex::migr::v4 {
inline cpp::result<bool, std::string> MigrateFolderStructureUp() {
  return true;
}

inline cpp::result<bool, std::string> MigrateFolderStructureDown() {
  return true;
}

// Database
inline cpp::result<bool, std::string> MigrateDBUp(SQLite::Database& db) {
  try {
    db.exec(
        "CREATE TABLE IF NOT EXISTS schema_version ( version INTEGER PRIMARY "
        "KEY);");

    // runs
    {
      // Check if the table exists
      SQLite::Statement query(db,
                              "SELECT name FROM sqlite_master WHERE "
                              "type='table' AND name='runs'");
      auto table_exists = query.executeStep();

      if (!table_exists) {
        // Create new table
        db.exec(
            "CREATE TABLE runs ("
            "id TEXT PRIMARY KEY,"
            "object TEXT,"
            "created_at INTEGER,"
            "assistant_id TEXT,"
            "thread_id TEXT,"
            "status TEXT,"
            "started_at INTEGER,"
            "expired_at INTEGER,"
            "cancelled_at INTEGER,"
            "failed_at INTEGER,"
            "completed_at INTEGER,"
            "last_error TEXT,"
            "model TEXT,"
            "instructions TEXT,"
            "tools TEXT,"
            "metadata TEXT,"
            "incomplete_details TEXT,"
            "usage TEXT,"
            "temperature REAL,"
            "top_p REAL,"
            "max_prompt_tokens INTEGER,"
            "max_completion_tokens INTEGER,"
            "truncation_strategy TEXT,"
            "response_format TEXT,"
            "tool_choice TEXT,"
            "parallel_tool_calls BOOL"
            ")");
      }
    }

    return true;
  } catch (const std::exception& e) {
    CTL_WRN("Migration up failed: " << e.what());
    return cpp::fail(e.what());
  }
};

inline cpp::result<bool, std::string> MigrateDBDown(SQLite::Database& db) {
  try {
    // runs
    {
      SQLite::Statement query(db,
                              "SELECT name FROM sqlite_master WHERE "
                              "type='table' AND name='runs'");
      auto table_exists = query.executeStep();
      if (table_exists) {
        db.exec("DROP TABLE runs");
      }
    }

    return true;
  } catch (const std::exception& e) {
    CTL_WRN("Migration down failed: " << e.what());
    return cpp::fail(e.what());
  }
}
};  // namespace cortex::migr::v4
