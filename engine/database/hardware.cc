#include "hardware.h"
#include "database.h"
#include "utils/logging_utils.h"
#include "utils/scope_exit.h"

namespace cortex::db {

Hardware::Hardware() : db_(cortex::db::Database::GetInstance().db()) {}

Hardware::Hardware(SQLite::Database& db) : db_(db) {}


Hardware::~Hardware() {}

cpp::result<std::vector<HardwareEntry>, std::string>
Hardware::LoadHardwareList() const {
  try {
    db_.exec("BEGIN TRANSACTION;");
    cortex::utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    std::vector<HardwareEntry> entries;
    SQLite::Statement query(
        db_,
        "SELECT uuid, type, "
        "hardware_id, software_id, activated, priority FROM hardware");

    while (query.executeStep()) {
      HardwareEntry entry;
      entry.uuid = query.getColumn(0).getString();
      entry.type = query.getColumn(1).getString();
      entry.hardware_id = query.getColumn(2).getInt();
      entry.software_id = query.getColumn(3).getInt();
      entry.activated = query.getColumn(4).getInt();
      entry.priority = query.getColumn(5).getInt();
      entries.push_back(entry);
    }
    return entries;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}
cpp::result<bool, std::string> Hardware::AddHardwareEntry(
    const HardwareEntry& new_entry) {
  try {
    SQLite::Statement insert(
        db_,
        "INSERT INTO hardware (uuid, type, "
        "hardware_id, software_id, activated, priority) VALUES (?, ?, "
        "?, ?, ?, ?)");
    insert.bind(1, new_entry.uuid);
    insert.bind(2, new_entry.type);
    insert.bind(3, new_entry.hardware_id);
    insert.bind(4, new_entry.software_id);
    insert.bind(5, new_entry.activated);
    insert.bind(6, new_entry.priority);
    insert.exec();
    CTL_INF("Inserted: " << new_entry.ToJsonString());
    return true;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}
cpp::result<bool, std::string> Hardware::UpdateHardwareEntry(
    const std::string& id, const HardwareEntry& updated_entry) {
  try {
    SQLite::Statement upd(
        db_,
        "UPDATE hardware "
        "SET hardware_id = ?, software_id = ?, activated = ?, priority = ? "
        "WHERE uuid = ?");
    upd.bind(1, updated_entry.hardware_id);
    upd.bind(2, updated_entry.software_id);
    upd.bind(3, updated_entry.activated);
    upd.bind(4, updated_entry.priority);
    upd.bind(5, id);
    if (upd.exec() == 1) {
      CTL_INF("Updated: " << updated_entry.ToJsonString());
      return true;
    }
    return false;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

cpp::result<bool, std::string> Hardware::DeleteHardwareEntry(
    const std::string& id) {
  try {
    SQLite::Statement del(db_, "DELETE from hardware WHERE uuid = ?");
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

bool Hardware::HasHardwareEntry(const std::string& id) {
   try {
    SQLite::Statement query(db_,
                            "SELECT COUNT(*) FROM hardware WHERE uuid = ?");
    query.bind(1, id);
    if (query.executeStep()) {
      return query.getColumn(0).getInt() > 0;
    }
    return false;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return false;
  }
}

cpp::result<bool, std::string> Hardware::UpdateHardwareEntry(const std::string& id,
                                                     int hw_id,
                                                     int sw_id) const {
 try {
    SQLite::Statement upd(
        db_,
        "UPDATE hardware "
        "SET hardware_id = ?, software_id = ? "
        "WHERE uuid = ?");
    upd.bind(1, hw_id);
    upd.bind(2, sw_id);
    upd.bind(3, id);
    if (upd.exec() == 1) {
      CTL_INF("Updated: " << id << " " << hw_id << " " << sw_id);
      return true;
    }
    return false;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
                                                     }
}  // namespace cortex::db
