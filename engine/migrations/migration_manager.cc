#include "migration_manager.h"
#include <filesystem>
#include "assert.h"
#include "schema_version.h"
#include "utils/file_manager_utils.h"
#include "utils/scope_exit.h"
#include "utils/widechar_conv.h"
#include "v0/migration.h"
#include "v1/migration.h"
#include "v2/migration.h"
#include "v3/migration.h"

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
    // CTL_WRN("SQLite error: " << e.what());
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
    auto src_db_path = (fmu::GetCortexDataPath() / kCortexDb);
    auto backup_db_path = (fmu::GetCortexDataPath() / kCortexDbBackup);
#if defined(_WIN32)
    if (auto res = mgr_helper_.BackupDatabase(
            src_db_path, cortex::wc::WstringToUtf8(backup_db_path.wstring()));
#else
    if (auto res =
            mgr_helper_.BackupDatabase(src_db_path, backup_db_path.string());
#endif
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
        (void)n;
        // CTL_INF("Deleted " << n << " files or directories");
      } catch (const std::exception& e) {
        CTL_WRN(e.what());
      }
    }
  });

  auto restore_db = [this]() -> cpp::result<bool, std::string> {
    auto src_db_path = (fmu::GetCortexDataPath() / kCortexDb);
    auto backup_db_path = (fmu::GetCortexDataPath() / kCortexDbBackup);
    return mgr_helper_.BackupDatabase(src_db_path, backup_db_path.string());
  };

  // Backup folder structure
  // Update logic if the folder structure changes

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
  // Update logic if the folder structure changes

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
  assert(current < target);
  for (int v = current + 1; v <= target; v++) {
    if (auto res = DoUpFolderStructure(v /*version*/); res.has_error()) {
      return res;
    }
  }
  return true;
}

cpp::result<bool, std::string> MigrationManager::DownFolderStructure(
    int current, int target) {
  assert(current > target);
  for (int v = current; v > target; v--) {
    if (auto res = DoDownFolderStructure(v /*version*/); res.has_error()) {
      return res;
    }
  }
  return true;
}

cpp::result<bool, std::string> MigrationManager::DoUpFolderStructure(
    int version) {
  switch (version) {
    case 0:
      return v0::MigrateFolderStructureUp();
    case 1:
      return v1::MigrateFolderStructureUp();
    case 2:
      return v2::MigrateFolderStructureUp();
    case 3:
      return v3::MigrateFolderStructureUp();

    default:
      return true;
  }
}
cpp::result<bool, std::string> MigrationManager::DoDownFolderStructure(
    int version) {
  switch (version) {
    case 0:
      return v0::MigrateFolderStructureDown();
    case 1:
      return v1::MigrateFolderStructureDown();
    case 2:
      return v2::MigrateFolderStructureDown();
    case 3:
      return v3::MigrateFolderStructureDown();

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
  return UpdateSchemaVersion(current, target);
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
  return UpdateSchemaVersion(current, target);
}

cpp::result<bool, std::string> MigrationManager::DoUpDB(int version) {
  switch (version) {
    case 0:
      return v0::MigrateDBUp(db_);
    case 1:
      return v1::MigrateDBUp(db_);
    case 2:
      return v2::MigrateDBUp(db_);
    case 3:
      return v3::MigrateDBUp(db_);

    default:
      return true;
  }
}

cpp::result<bool, std::string> MigrationManager::DoDownDB(int version) {
  switch (version) {
    case 0:
      return v0::MigrateDBDown(db_);
    case 1:
      return v1::MigrateDBDown(db_);
    case 2:
      return v2::MigrateDBDown(db_);
    case 3:
      return v3::MigrateDBDown(db_);

    default:
      return true;
  }
}

cpp::result<bool, std::string> MigrationManager::UpdateSchemaVersion(
    int old_version, int new_version) {
  if (old_version == new_version)
    return true;
  try {
    db_.exec("BEGIN TRANSACTION;");

    SQLite::Statement insert(db_,
                             "INSERT INTO schema_version (version) VALUES (?)");
    insert.bind(1, new_version);
    insert.exec();

    if (old_version != -1) {
      SQLite::Statement del(db_,
                            "DELETE FROM schema_version WHERE version = ?");
      del.bind(1, old_version);
      del.exec();
    }

    db_.exec("COMMIT;");
    // CTL_INF("Inserted: " << version);
    return true;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}
}  // namespace cortex::migr
