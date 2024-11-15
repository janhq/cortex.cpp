#pragma once
#include <SQLiteCpp/Database.h>
#include <filesystem>
#include <string>
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"

namespace cortex::migr::v0 {
// Data folder
namespace fmu = file_manager_utils;

// cortexcpp
//   |__ models
//   |     |__ cortex.so
//   |          |__ tinyllama
//   |                |__ gguf
//   |__ engines
//   |     |__ cortex.llamacpp
//   |           |__ deps
//   |           |__ windows-amd64-avx
//   |__ logs
//
inline cpp::result<bool, std::string> MigrateFolderStructureUp() {
  if (!std::filesystem::exists(fmu::GetCortexDataPath() / "models")) {
    std::filesystem::create_directory(fmu::GetCortexDataPath() / "models");
  }

  if (!std::filesystem::exists(fmu::GetCortexDataPath() / "engines")) {
    std::filesystem::create_directory(fmu::GetCortexDataPath() / "engines");
  }

  if (!std::filesystem::exists(fmu::GetCortexDataPath() / "logs")) {
    std::filesystem::create_directory(fmu::GetCortexDataPath() / "logs");
  }

  return true;
}

inline cpp::result<bool, std::string> MigrateFolderStructureDown() {
  CTL_INF("Folder structure already up to date!");
  return true;
}

// Database
inline cpp::result<bool, std::string> MigrateDBUp(SQLite::Database& db) {
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

inline cpp::result<bool, std::string> MigrateDBDown(SQLite::Database& db) {
  try {
    db.exec("DROP TABLE IF EXISTS hardware;");
    db.exec("DROP TABLE IF EXISTS models;");
    CTL_INF("Migration down completed successfully.");
    return true;
  } catch (const std::exception& e) {
    CTL_WRN("Migration down failed: " << e.what());
    return cpp::fail(e.what());
  }
}

};  // namespace cortex::migr::v0
