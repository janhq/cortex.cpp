#include "hardwares.h"
#include "database.h"
#include "utils/scope_exit.h"

namespace cortex::db {

Hardwares::Hardwares() : db_(cortex::db::Database::GetInstance().db()) {
  db_.exec(
      "CREATE TABLE IF NOT EXISTS hardwares ("
      "uuid TEXT PRIMARY KEY,"
      "type TEXT,"
      "hardware_id INTEGER,"
      "software_id INTEGER,"
      "activated INTEGER);");
}

Hardwares::Hardwares(SQLite::Database& db) : db_(db) {
  db_.exec(
      "CREATE TABLE IF NOT EXISTS hardwares ("
      "uuid TEXT PRIMARY KEY,"
      "type TEXT,"
      "hardware_id INTEGER,"
      "software_id INTEGER,"
      "activated INTEGER);");
}

Hardwares::~Hardwares() {}

cpp::result<std::vector<HardwareEntry>, std::string>
Hardwares::LoadHardwareList() const {
  try {
    db_.exec("BEGIN TRANSACTION;");
    cortex::utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    std::vector<HardwareEntry> entries;
    SQLite::Statement query(
        db_,
        "SELECT uuid, type, "
        "hardware_id, software_id, activated FROM hardwares");

    while (query.executeStep()) {
      HardwareEntry entry;
      entry.uuid = query.getColumn(0).getString();
      entry.type = query.getColumn(1).getString();
      entry.hardware_id = query.getColumn(2).getInt();
      entry.software_id = query.getColumn(3).getInt();
      entry.activated = query.getColumn(4).getInt();
      entries.push_back(entry);
    }
    return entries;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}
cpp::result<bool, std::string> Hardwares::AddHardwareEntry(
    const HardwareEntry& new_entry) {
  try {
    SQLite::Statement insert(
        db_,
        "INSERT INTO hardwares (uuid, type, "
        "hardware_id, software_id, activated) VALUES (?, ?, "
        "?, ?, ?)");
    insert.bind(1, new_entry.uuid);
    insert.bind(2, new_entry.type);
    insert.bind(3, new_entry.hardware_id);
    insert.bind(4, new_entry.software_id);
    insert.bind(5, new_entry.activated);
    insert.exec();
    CTL_INF("Inserted: " << new_entry.ToJsonString());
    return true;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}
cpp::result<bool, std::string> Hardwares::UpdateHardwareEntry(
    const std::string& id, const HardwareEntry& updated_entry) {
  try {
    SQLite::Statement upd(db_,
                          "UPDATE hardwares "
                          "SET hardware_id = ?, software_id = ?, activated = ? "
                          "WHERE uuid = ?");
    upd.bind(1, updated_entry.hardware_id);
    upd.bind(2, updated_entry.software_id);
    upd.bind(3, updated_entry.activated);
    upd.bind(4, id);
    if (upd.exec() == 1) {
      CTL_INF("Updated: " << updated_entry.ToJsonString());
      return true;
    }
    return false;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

cpp::result<bool, std::string> Hardwares::DeleteHardwareEntry(
    const std::string& id) {
  try {
    SQLite::Statement del(db_, "DELETE from hardwares WHERE uuid = ?");
    del.bind(1, id);
    if (del.exec() == 1) {
      CTL_INF("Deleted: " << id);
      return true;
    }
    return false;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}
}  // namespace cortex::db