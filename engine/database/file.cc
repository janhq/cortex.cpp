#include "file.h"
#include "utils/logging_utils.h"
#include "utils/scope_exit.h"

namespace cortex::db {

cpp::result<std::vector<OpenAi::File>, std::string> File::GetFileList() const {
  try {
    db_.exec("BEGIN TRANSACTION;");
    cortex::utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    std::vector<OpenAi::File> entries;
    SQLite::Statement query(db_,
                            "SELECT id, object, "
                            "purpose, filename, created_at, bytes FROM files");

    while (query.executeStep()) {
      OpenAi::File entry;
      entry.id = query.getColumn(0).getString();
      entry.object = query.getColumn(1).getString();
      entry.purpose = query.getColumn(2).getString();
      entry.filename = query.getColumn(3).getString();
      entry.created_at = query.getColumn(4).getInt();
      entry.bytes = query.getColumn(5).getInt();
      entries.push_back(entry);
    }
    return entries;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

cpp::result<OpenAi::File, std::string> File::GetFileById(
    const std::string& file_id) const {
  try {
    SQLite::Statement query(db_,
                            "SELECT id, object, "
                            "purpose, filename, created_at, bytes FROM files "
                            "WHERE id = ?");

    query.bind(1, file_id);
    if (query.executeStep()) {
      OpenAi::File entry;
      entry.id = query.getColumn(0).getString();
      entry.object = query.getColumn(1).getString();
      entry.purpose = query.getColumn(2).getString();
      entry.filename = query.getColumn(3).getString();
      entry.created_at = query.getColumn(4).getInt();
      entry.bytes = query.getColumn(5).getInt64();
      return entry;
    } else {
      return cpp::fail("File not found: " + file_id);
    }
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

cpp::result<void, std::string> File::AddFileEntry(OpenAi::File& file) {
  try {
    SQLite::Statement insert(
        db_,
        "INSERT INTO files (id, object, "
        "purpose, filename, created_at, bytes) VALUES (?, ?, "
        "?, ?, ?, ?)");
    insert.bind(1, file.id);
    insert.bind(2, file.object);
    insert.bind(3, file.purpose);
    insert.bind(4, file.filename);
    insert.bind(5, std::to_string(file.created_at));
    insert.bind(6, std::to_string(file.bytes));
    insert.exec();

    CTL_INF("Inserted: " << file.ToJson()->toStyledString());
    return {};
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

cpp::result<void, std::string> File::DeleteFileEntry(
    const std::string& file_id) {
  try {
    SQLite::Statement del(db_, "DELETE from files WHERE id = ?");
    del.bind(1, file_id);
    if (del.exec() == 1) {
      CTL_INF("Deleted: " << file_id);
      return {};
    }
    return {};
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}
}  // namespace cortex::db
