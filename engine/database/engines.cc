#include "engines.h"
#include <optional>
#include <SQLiteCpp/Database.h>
#include "database.h"

namespace cortex::db {

void CreateTable(SQLite::Database& db) {
    db.exec(
        "CREATE TABLE IF NOT EXISTS engines ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "engine_name TEXT,"
        "type TEXT,"
        "api_key TEXT,"
        "url TEXT,"
        "version TEXT,"
        "variant TEXT,"
        "status TEXT,"
        "metadata TEXT,"
        "date_created TEXT DEFAULT CURRENT_TIMESTAMP,"
        "date_updated TEXT DEFAULT CURRENT_TIMESTAMP,"
        "UNIQUE(engine_name, variant));");  // Add UNIQUE constraint
}

Engines::Engines() : db_(cortex::db::Database::GetInstance().db()) {
    CreateTable(db_);
}

Engines::Engines(SQLite::Database& db) : db_(db) {
    CreateTable(db_);
}

Engines::~Engines() {}

// Function to create a new engine and save it into the database
std::optional<std::string> Engines::UpsertEngine(const std::string& engine_name,
                                                 const std::string& type,
                                                 const std::string& api_key,
                                                 const std::string& url,
                                                 const std::string& version,
                                                 const std::string& variant,
                                                 const std::string& status,
                                                 const std::string& metadata) {
    try {
        SQLite::Statement query(db_,
            "INSERT INTO engines (engine_name, type, api_key, url, version, variant, status, metadata) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?) "
            "ON CONFLICT(engine_name, variant) DO UPDATE SET "
            "type = excluded.type, "
            "api_key = excluded.api_key, "
            "url = excluded.url, "
            "version = excluded.version, "
            "status = excluded.status, "
            "metadata = excluded.metadata, "
            "date_updated = CURRENT_TIMESTAMP;");

        query.bind(1, engine_name);
        query.bind(2, type);
        query.bind(3, api_key);
        query.bind(4, url);
        query.bind(5, version);
        query.bind(6, variant);
        query.bind(7, status);
        query.bind(8, metadata);

        query.exec();
        return std::nullopt;
    } catch (const std::exception& e) {
        return std::string("Failed to upsert engine: ") + e.what();
    }
}

std::optional<EngineEntry> Engines::GetEngine(int id, const std::string& engine_name) const {
    try {
        SQLite::Statement query(db_,
            "SELECT engine_name FROM engines "
            "WHERE (id = ? OR engine_name = ?) AND status = 'Default' "
            "ORDER BY date_updated DESC LIMIT 1");

        query.bind(1, id);
        query.bind(2, engine_name);

        if (query.executeStep()) {
            return EngineEntry{query.getColumn(0).getString()};
        } else {
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::optional<std::string> Engines::DeleteEngine(int id) {
    try {
        SQLite::Statement query(db_,
            "DELETE FROM engines WHERE id = ?");

        query.bind(1, id);
        query.exec();
        return std::nullopt;
    } catch (const std::exception& e) {
        return std::string("Failed to delete engine: ") + e.what();
    }
}

}  // namespace cortex::db