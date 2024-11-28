#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <sqlite3.h>
#include <filesystem>
#include "utils/logging_utils.h"
#include "utils/result.hpp"

namespace cortex::migr {
class MigrationHelper {
 public:
  cpp::result<bool, std::string> BackupDatabase(
      const std::filesystem::path& src_db_path,
      const std::string& backup_db_path);

  cpp::result<bool, std::string> RestoreDatabase(
      const std::string& backup_db_path,
      const std::filesystem::path& target_db_path);
};
}  // namespace cortex::migr
