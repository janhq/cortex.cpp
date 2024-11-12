#include "engines.h"
#include "database.h"

namespace cortex::db {

Engines::Engines() : db_(cortex::db::Database::GetInstance().db()) {
  db_.exec(
      "CREATE TABLE IF NOT EXISTS engines ("
      "engine_id TEXT PRIMARY KEY,"
      "type TEXT,"
      "api_key TEXT,"
      "url TEXT,"
      "version TEXT,"
      "variant TEXT,"
      "status TEXT,"
      "metadata TEXT);");
}
Engines::~Engines() {}
}