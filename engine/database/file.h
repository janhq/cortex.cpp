#pragma once

#include <SQLiteCpp/Database.h>
#include <trantor/utils/Logger.h>
#include <string>
#include <vector>
#include "common/file.h"
#include "database.h"
#include "utils/result.hpp"

namespace cortex::db {
class File {
  SQLite::Database& db_;

 public:
  File(SQLite::Database& db) : db_{db} {};

  File() : db_(cortex::db::Database::GetInstance().db()) {}

  ~File() {}

  cpp::result<std::vector<OpenAi::File>, std::string> GetFileList() const;

  cpp::result<OpenAi::File, std::string> GetFileById(
      const std::string& file_id) const;

  cpp::result<void, std::string> AddFileEntry(OpenAi::File& file);

  cpp::result<void, std::string> DeleteFileEntry(const std::string& file_id);
};
}  // namespace cortex::db
