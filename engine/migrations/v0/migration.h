#pragma once
#include <SQLiteCpp/Database.h>
#include <string>
#include "utils/logging_utils.h"
#include "utils/result.hpp"

namespace cortex::migr::v0 {
// Data folder

inline cpp::result<bool, std::string> MigrateUp() {
  return true;
}

inline cpp::result<bool, std::string> MigrateDown() {
  return true;
}

// Database
inline cpp::result<bool, std::string> MigrateUp(SQLite::Database& db) {
  try {
    db.exec(
        "CREATE TABLE IF NOT EXISTS schema_version ( version INTEGER NOT "
        "NULL);");

    db.exec(
        "CREATE TABLE IF NOT EXISTS models ("
        "model_id TEXT PRIMARY KEY,"
        "author_repo_id TEXT,"
        "branch_name TEXT,"
        "path_to_model_yaml TEXT,"
        "model_alias TEXT);");

    db.exec(
        "CREATE TABLE IF NOT EXISTS hardware ("
        "uuid TEXT NOT NULL, "
        "type TEXT NOT NULL, "
        "hardware_id INTEGER NOT NULL, "
        "software_id INTEGER NOT NULL, "
        "activated INTEGER NOT NULL CHECK (activated IN (0, 1)));");

    CTL_INF("Database migration up completed successfully.");
    return true;
  } catch (const std::exception& e) {
    CTL_WRN("Migration up failed: " << e.what());
    return cpp::fail(e.what());
  }
};

inline cpp::result<bool, std::string> MigrateDown(SQLite::Database& db) {
  CTL_INF("No need to drop tables for version 0");
  return true;
}

};  // namespace cortex::migr::v0
