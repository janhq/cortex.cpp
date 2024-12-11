#pragma once

#include "migration_helper.h"

namespace cortex::migr {
class MigrationManager {
 public:
  explicit MigrationManager(SQLite::Database& db) : db_(db) {}
  cpp::result<bool, std::string> Migrate();

 private:
  cpp::result<bool, std::string> UpFolderStructure(int current, int target);
  cpp::result<bool, std::string> DownFolderStructure(int current, int target);

  cpp::result<bool, std::string> DoUpFolderStructure(int version);
  cpp::result<bool, std::string> DoDownFolderStructure(int version);

  cpp::result<bool, std::string> UpDB(int current, int target);
  cpp::result<bool, std::string> DownDB(int current, int target);

  cpp::result<bool, std::string> DoUpDB(int version);
  cpp::result<bool, std::string> DoDownDB(int version);

  cpp::result<bool, std::string> UpdateSchemaVersion(int old_version,
                                                     int new_version);

 private:
  MigrationHelper mgr_helper_;
  SQLite::Database& db_;
};
}  // namespace cortex::migr
