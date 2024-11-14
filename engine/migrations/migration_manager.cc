#include "migration_manager.h"
#include "schema_version.h"

namespace cortex::migr {

namespace {
cpp::result<bool, std::string> DoUp(int version) {
  switch (version) {
    case 0:
      return v0::MigrateUp();
      break;

    default:
      return true;
  }
}

cpp::result<bool, std::string> DoDown(int version) {
  switch (version) {
    case 0:
      return v0::MigrateDown();
      break;

    default:
      return true;
  }
}

int GetSchemaVersion(SQLite::Database& db) {
  int version = 0;  // Default version if not set

  try {
    SQLite::Statement query(db, "SELECT version FROM schema_version LIMIT 1;");

    // Execute the query and get the result
    if (query.executeStep()) {
      version =
          query.getColumn(0).getInt();  // Get the version from the first column
    }
  } catch (const std::exception& e) {
    std::cerr << "SQLite error: " << e.what() << std::endl;
    // Handle exceptions, possibly setting a default version or taking corrective action
  }

  return version;
}
}  // namespace

cpp::result<bool, std::string> MigrationManager::Up(SQLite::Database& db) {
  // check schema db version
  int schema_version = GetSchemaVersion(db);
  int current_version = SCHEMA_VERSION;
  for (int i = schema_version; i <= current_version; i++) {
    if (auto res = DoUp(i /*version*/); res.has_error()) {
      // Restore db and file structure
    }
  }
  return true;
}
cpp::result<bool, std::string> MigrationManager::Down(SQLite::Database& db) {
  return true;
}
}  // namespace cortex::migr