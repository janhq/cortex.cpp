#pragma once
#include <SQLiteCpp/Database.h>
#include <filesystem>
#include <string>
#include "migrations/db_helper.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"

namespace cortex::migr::v2 {
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
  // CTL_INF("Folder structure already up to date!");
  return true;
}

// Database
inline cpp::result<bool, std::string> MigrateDBUp(SQLite::Database& db) {
  try {
    db.exec(
        "CREATE TABLE IF NOT EXISTS schema_version ( version INTEGER PRIMARY "
        "KEY);");

    // models
    {
      // Check if the table exists
      SQLite::Statement query(db,
                              "SELECT name FROM sqlite_master WHERE "
                              "type='table' AND name='models'");
      auto table_exists = query.executeStep();

      if (table_exists) {
        // Alter existing table
        cortex::mgr::AddColumnIfNotExists(db, "models", "model_format", "TEXT");
        cortex::mgr::AddColumnIfNotExists(db, "models", "model_source", "TEXT");
        cortex::mgr::AddColumnIfNotExists(db, "models", "status", "TEXT");
        cortex::mgr::AddColumnIfNotExists(db, "models", "engine", "TEXT");
      } else {
        // Create new table
        db.exec(
            "CREATE TABLE models ("
            "model_id TEXT PRIMARY KEY,"
            "author_repo_id TEXT,"
            "branch_name TEXT,"
            "path_to_model_yaml TEXT,"
            "model_alias TEXT,"
            "model_format TEXT,"
            "model_source TEXT,"
            "status TEXT,"
            "engine TEXT"
            ")");
      }
    }

    // Check if the table exists
    SQLite::Statement hw_query(db,
                               "SELECT name FROM sqlite_master WHERE "
                               "type='table' AND name='hardware'");
    auto hw_table_exists = hw_query.executeStep();

    if (hw_table_exists) {
      // Alter existing table
      cortex::mgr::AddColumnIfNotExists(db, "hardware", "priority", "INTEGER");
    } else {
      db.exec(
          "CREATE TABLE IF NOT EXISTS hardware ("
          "uuid TEXT PRIMARY KEY, "
          "type TEXT NOT NULL, "
          "hardware_id INTEGER NOT NULL, "
          "software_id INTEGER NOT NULL, "
          "activated INTEGER NOT NULL CHECK (activated IN (0, 1)), "
          "priority INTEGER); ");
    }

    // engines
    db.exec(
        "CREATE TABLE IF NOT EXISTS engines ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "engine_name TEXT,"
        "type TEXT,"
        "api_key TEXT,"
        "url TEXT,"
        "version TEXT,"
        "variant TEXT,"
        "status TEXT,"
        "metadata TEXT,"
        "date_created TEXT DEFAULT CURRENT_TIMESTAMP,"
        "date_updated TEXT DEFAULT CURRENT_TIMESTAMP,"
        "UNIQUE(engine_name, variant));");

    // CTL_INF("Database migration up completed successfully.");
    return true;
  } catch (const std::exception& e) {
    CTL_WRN("Migration up failed: " << e.what());
    return cpp::fail(e.what());
  }
};

inline cpp::result<bool, std::string> MigrateDBDown(SQLite::Database& db) {
  try {
    // models
    {
      SQLite::Statement query(db,
                              "SELECT name FROM sqlite_master WHERE "
                              "type='table' AND name='models'");
      auto table_exists = query.executeStep();
      if (table_exists) {
        // Create a new table with the old schema
        db.exec(
            "CREATE TABLE models_old ("
            "model_id TEXT PRIMARY KEY,"
            "author_repo_id TEXT,"
            "branch_name TEXT,"
            "path_to_model_yaml TEXT,"
            "model_alias TEXT"
            ")");

        // Copy data from the current table to the new table
        db.exec(
            "INSERT INTO models_old (model_id, author_repo_id, branch_name, "
            "path_to_model_yaml, model_alias) "
            "SELECT model_id, author_repo_id, branch_name, path_to_model_yaml, "
            "model_alias FROM models");

        // Drop the current table
        db.exec("DROP TABLE models");

        // Rename the new table to the original name
        db.exec("ALTER TABLE models_old RENAME TO models");
      }
    }

    // hardware
    {
      SQLite::Statement query(db,
                              "SELECT name FROM sqlite_master WHERE "
                              "type='table' AND name='hardware'");
      auto table_exists = query.executeStep();
      if (table_exists) {
        // Create a new table with the old schema
        db.exec(
            "CREATE TABLE hardware_old ("
            "uuid TEXT PRIMARY KEY, "
            "type TEXT NOT NULL, "
            "hardware_id INTEGER NOT NULL, "
            "software_id INTEGER NOT NULL, "
            "activated INTEGER NOT NULL CHECK (activated IN (0, 1))"
            ")");

        // Copy data from the current table to the new table
        db.exec(
            "INSERT INTO hardware_old (uuid, type, hardware_id, "
            "software_id, activated) "
            "SELECT uuid, type, hardware_id, software_id, "
            "activated FROM hardware");

        // Drop the current table
        db.exec("DROP TABLE hardware");

        // Rename the new table to the original name
        db.exec("ALTER TABLE hardware_old RENAME TO hardware");
      }
    }

    // engines
    {
      // do nothing
    }
    // CTL_INF("Migration down completed successfully.");
    return true;
  } catch (const std::exception& e) {
    CTL_WRN("Migration down failed: " << e.what());
    return cpp::fail(e.what());
  }
}

};  // namespace cortex::migr::v2