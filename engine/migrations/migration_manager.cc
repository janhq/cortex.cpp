#include "migration_manager.h"
#include <filesystem>
#include "assert.h"
#include "schema_version.h"
#include "utils/file_manager_utils.h"
#include "utils/scope_exit.h"

namespace cortex::migr {

namespace {
int GetSchemaVersion(SQLite::Database& db) {
  int version = -1;  // Default version if not set

  try {
    SQLite::Statement query(db, "SELECT version FROM schema_version LIMIT 1;");

    // Execute the query and get the result
    if (query.executeStep()) {
      version =
          query.getColumn(0).getInt();  // Get the version from the first column
    }
  } catch (const std::exception& e) {
    CTL_WRN("SQLite error: " << e.what());
    // Handle exceptions, possibly setting a default version or taking corrective action
  }

  return version;
}

constexpr const auto kCortexDb = "cortex.db";
constexpr const auto kCortexDbBackup = "cortex_backup.db";
}  // namespace

cpp::result<bool, std::string> MigrationManager::Migrate() {
  namespace fmu = file_manager_utils;
  int last_schema_version = GetSchemaVersion(db_);
  int target_schema_version = SCHEMA_VERSION;
  if (last_schema_version == target_schema_version)
    return true;
  // Back up all data before migrating
  if (std::filesystem::exists(fmu::GetCortexDataPath() / kCortexDb)) {
    auto src_db_path = (fmu::GetCortexDataPath() / kCortexDb).string();
    auto backup_db_path = (fmu::GetCortexDataPath() / kCortexDbBackup).string();
    if (auto res = mgr_helper_.BackupDatabase(src_db_path, backup_db_path);
        res.has_error()) {
      CTL_INF("Error: backup database failed!");
      return res;
    }
  }

  cortex::utils::ScopeExit se([]() {
    auto cortex_tmp = fmu::GetCortexDataPath() / kCortexDbBackup;
    if (std::filesystem::exists(cortex_tmp)) {
      try {
        auto n = std::filesystem::remove_all(cortex_tmp);
        CTL_INF("Deleted " << n << " files or directories");
      } catch (const std::exception& e) {
        CTL_WRN(e.what());
      }
    }
  });

  auto restore_db = [this]() -> cpp::result<bool, std::string> {
    auto src_db_path = (fmu::GetCortexDataPath() / kCortexDb).string();
    auto backup_db_path = (fmu::GetCortexDataPath() / kCortexDbBackup).string();
    return mgr_helper_.BackupDatabase(src_db_path, backup_db_path);
  };

  // Backup folder structure
  // TODO(any) update logic if the folder structure changes

  // Migrate folder structure
  if (last_schema_version <= target_schema_version) {
    if (auto res =
            UpFolderStructure(last_schema_version, target_schema_version);
        res.has_error()) {
      // Restore
      return res;
    }
  } else {
    if (auto res =
            DownFolderStructure(last_schema_version, target_schema_version);
        res.has_error()) {
      // Restore
      return res;
    }
  }

  // Update database on folder structure changes
  // TODO(any) update logic if the folder structure changes

  // Migrate database
  if (last_schema_version < target_schema_version) {
    if (auto res = UpDB(last_schema_version, target_schema_version);
        res.has_error()) {
      auto r = restore_db();
      return res;
    }
  } else {
    if (auto res = DownDB(last_schema_version, target_schema_version);
        res.has_error()) {
      auto r = restore_db();
      return res;
    }
  }
  return true;
}

cpp::result<bool, std::string> MigrationManager::UpFolderStructure(int current,
                                                                   int target) {
  assert(current <= target);
  for (int i = current; i < target; i++) {
    if (auto res = DoUpFolderStructure(i /*version*/); res.has_error()) {
      // Restore db and file structure
    }
  }
  return true;
}
cpp::result<bool, std::string> MigrationManager::DownFolderStructure(
    int current, int target) {
  assert(current >= target);
  for (int i = current; i >= target; i--) {
    if (auto res = DoDownFolderStructure(i /*version*/); res.has_error()) {
      // Restore db and file structure
    }
  }
  return true;
}

cpp::result<bool, std::string> MigrationManager::DoUpFolderStructure(
    int version) {
  switch (version) {
    case 0:
      return v0::MigrateFolderStructureUp();
      break;

    default:
      return true;
  }
}
cpp::result<bool, std::string> MigrationManager::DoDownFolderStructure(
    int version) {
  switch (version) {
    case 0:
      return v0::MigrateFolderStructureDown();
      break;

    default:
      return true;
  }
}

cpp::result<bool, std::string> MigrationManager::UpDB(int current, int target) {
  assert(current < target);
  for (int v = current + 1; v <= target; v++) {
    if (auto res = DoUpDB(v /*version*/); res.has_error()) {
      return res;
    }
  }
  // Save database
  return UpdateSchemaVersion(target);
}
cpp::result<bool, std::string> MigrationManager::DownDB(int current,
                                                        int target) {
  assert(current > target);
  for (int v = current; v > target; v--) {
    if (auto res = DoDownDB(v /*version*/); res.has_error()) {
      return res;
    }
  }
  // Save database
  return UpdateSchemaVersion(target);
}

cpp::result<bool, std::string> MigrationManager::DoUpDB(int version) {
  switch (version) {
    case 0:
      return v0::MigrateDBUp(db_);
      break;

    default:
      return true;
  }
}

cpp::result<bool, std::string> MigrationManager::DoDownDB(int version) {
  switch (version) {
    case 0:
      return v0::MigrateDBDown(db_);
      break;

    default:
      return true;
  }
}

cpp::result<bool, std::string> MigrationManager::UpdateSchemaVersion(
    int version) {
  try {
    SQLite::Statement insert(db_,
                             "INSERT INTO schema_version (version) VALUES (?)");
    insert.bind(1, version);
    insert.exec();
    CTL_INF("Inserted: " << version);
    return true;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}
}  // namespace cortex::migr