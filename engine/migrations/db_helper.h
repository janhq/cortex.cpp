#pragma once
#include <SQLiteCpp/SQLiteCpp.h>

namespace cortex::mgr {
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

inline bool ColumnExists(SQLite::Database& db, const std::string& table_name, const std::string& column_name) {
    try {
        SQLite::Statement query(db, "SELECT " + column_name + " FROM " + table_name + " LIMIT 0");
        return true;
    } catch (std::exception&) {
        return false;
    }
}

inline void AddColumnIfNotExists(SQLite::Database& db, const std::string& table_name, 
                              const std::string& column_name, const std::string& column_type) {
    if (!ColumnExists(db, table_name, column_name)) {
        std::string sql = "ALTER TABLE " + table_name + " ADD COLUMN " + column_name + " " + column_type;
        db.exec(sql);
    }
}
}