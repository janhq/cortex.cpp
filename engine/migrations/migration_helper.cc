#include "migration_helper.h"

namespace cortex::migr {
cpp::result<bool, std::string> MigrationHelper::BackupDatabase(
    const std::filesystem::path& src_db_path,
    const std::string& backup_db_path) {
  try {
    SQLite::Database src_db(src_db_path, SQLite::OPEN_READONLY);
    sqlite3* backup_db;

    if (sqlite3_open(backup_db_path.c_str(), &backup_db) != SQLITE_OK) {
      throw std::runtime_error("Failed to open backup database");
    }

    sqlite3_backup* backup =
        sqlite3_backup_init(backup_db, "main", src_db.getHandle(), "main");
    if (!backup) {
      sqlite3_close(backup_db);
      throw std::runtime_error("Failed to initialize backup");
    }

    if (sqlite3_backup_step(backup, -1) != SQLITE_DONE) {
      sqlite3_backup_finish(backup);
      sqlite3_close(backup_db);
      throw std::runtime_error("Failed to perform backup");
    }

    sqlite3_backup_finish(backup);
    sqlite3_close(backup_db);
    // CTL_INF("Backup completed successfully.");
    return true;
  } catch (const std::exception& e) {
    CTL_WRN("Error during backup: " << e.what());
    return cpp::fail(e.what());
  }
}

cpp::result<bool, std::string> MigrationHelper::RestoreDatabase(
    const std::string& backup_db_path,
    const std::filesystem::path& target_db_path) {
  try {
    SQLite::Database target_db(target_db_path,
                               SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    sqlite3* backup_db;

    if (sqlite3_open16(backup_db_path.c_str(), &backup_db) != SQLITE_OK) {
      throw std::runtime_error("Failed to open backup database");
    }

    sqlite3_backup* backup =
        sqlite3_backup_init(target_db.getHandle(), "main", backup_db, "main");
    if (!backup) {
      sqlite3_close(backup_db);
      throw std::runtime_error("Failed to initialize restore");
    }

    if (sqlite3_backup_step(backup, -1) != SQLITE_DONE) {
      sqlite3_backup_finish(backup);
      sqlite3_close(backup_db);
      throw std::runtime_error("Failed to perform restore");
    }

    sqlite3_backup_finish(backup);
    sqlite3_close(backup_db);
    // CTL_INF("Restore completed successfully.");
    return true;
  } catch (const std::exception& e) {
    CTL_WRN("Error during restore: " << e.what());
    return cpp::fail(e.what());
  }
}
}  // namespace cortex::migr