#pragma once

#include <SQLiteCpp/Database.h>
#include <string>
#include "utils/logging_utils.h"
#include "utils/result.hpp"

namespace cortex::migr::v3 {
inline cpp::result<bool, std::string> MigrateFolderStructureUp() {
  return true;
}

inline cpp::result<bool, std::string> MigrateFolderStructureDown() {
  // CTL_INF("Folder structure already up to date!");
  return true;
}

// Database
inline cpp::result<bool, std::string> MigrateDBUp(SQLite::Database& db) {
  try {
    db.exec(
        "CREATE TABLE IF NOT EXISTS schema_version ( version INTEGER PRIMARY "
        "KEY);");

    // files
    {
      // Check if the table exists
      SQLite::Statement query(db,
                              "SELECT name FROM sqlite_master WHERE "
                              "type='table' AND name='files'");
      auto table_exists = query.executeStep();

      if (!table_exists) {
        // Create new table
        db.exec(
            "CREATE TABLE files ("
            "id TEXT PRIMARY KEY,"
            "object TEXT,"
            "purpose TEXT,"
            "filename TEXT,"
            "created_at INTEGER,"
            "bytes INTEGER"
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
    // hardware
    {
      SQLite::Statement query(db,
                              "SELECT name FROM sqlite_master WHERE "
                              "type='table' AND name='hardware'");
      auto table_exists = query.executeStep();
      if (table_exists) {
        db.exec("DROP TABLE files");
      }
    }

    return true;
  } catch (const std::exception& e) {
    CTL_WRN("Migration down failed: " << e.what());
    return cpp::fail(e.what());
  }
}
};  // namespace cortex::migr::v3
