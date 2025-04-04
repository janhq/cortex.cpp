#pragma once

#include <SQLiteCpp/Database.h>
#include <json/json.h>
#include <json/value.h>
#include <trantor/utils/Logger.h>
#include <optional>
#include <string>
#include <vector>

namespace cortex::db {

struct EngineEntry {
  int id;
  std::string engine_name;
  std::string type;
  std::string api_key;
  std::string url;
  std::string version;
  std::string variant;
  std::string status;
  std::string metadata;
  std::string date_created;
  std::string date_updated;
  Json::Value ToJson() const {
    Json::Value root;
    Json::Reader reader;

    // Convert basic fields
    root["id"] = id;
    root["engine"] = engine_name;
    root["type"] = type;
    root["api_key"] = api_key;
    root["url"] = url;
    root["version"] = version;
    root["variant"] = variant;
    root["status"] = status;
    root["date_created"] = date_created;
    root["date_updated"] = date_updated;

    // Parse metadata string into JSON object
    Json::Value metadataJson;
    if (!metadata.empty()) {
      bool success = reader.parse(metadata, metadataJson,
                                  false);  // false = don't collect comments
      if (success) {
        root["metadata"] = metadataJson;
      } else {
        root["metadata"] = Json::Value(Json::nullValue);
      }
    } else {
      root["metadata"] = Json::Value(Json::objectValue);  // empty object
    }

    return root;
  }
};

class Engines {
 private:
  SQLite::Database& db_;

  bool IsUnique(const std::vector<EngineEntry>& entries,
                const std::string& model_id,
                const std::string& model_alias) const;

  std::optional<std::vector<EngineEntry>> LoadModelListNoLock() const;

 public:
  Engines();
  Engines(SQLite::Database& db);
  ~Engines();

  std::optional<EngineEntry> UpsertEngine(
      const std::string& engine_name, const std::string& type,
      const std::string& api_key, const std::string& url,
      const std::string& version, const std::string& variant,
      const std::string& status, const std::string& metadata);

  std::optional<std::vector<EngineEntry>> GetEngines() const;
  std::optional<EngineEntry> GetEngineById(int id) const;
  std::optional<EngineEntry> GetEngineByNameAndVariant(
      const std::string& engine_name,
      const std::optional<std::string> variant = std::nullopt) const;

  std::optional<std::string> DeleteEngineById(int id);
};

}  // namespace cortex::db