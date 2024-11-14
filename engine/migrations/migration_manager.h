#pragma once
#include "v0/migration.h"

namespace cortex::migr {
class MigrationManager {
  cpp::result<bool, std::string> Up(SQLite::Database& db);
  cpp::result<bool, std::string> Down(SQLite::Database& db);
};
}  // namespace cortex::migr