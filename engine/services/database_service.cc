#include "database_service.h"

// engines
std::optional<EngineEntry> DatabaseService::UpsertEngine(
    const std::string& engine_name, const std::string& type,
    const std::string& api_key, const std::string& url,
    const std::string& version, const std::string& variant,
    const std::string& status, const std::string& metadata) {
  return cortex::db::Engines().UpsertEngine(engine_name, type, api_key, url,
                                            version, variant, status, metadata);
}

std::optional<std::vector<EngineEntry>> DatabaseService::GetEngines() const {
  return cortex::db::Engines().GetEngines();
}

std::optional<EngineEntry> DatabaseService::GetEngineById(int id) const {
  return cortex::db::Engines().GetEngineById(id);
}

std::optional<EngineEntry> DatabaseService::GetEngineByNameAndVariant(
    const std::string& engine_name,
    const std::optional<std::string> variant) const {
  return cortex::db::Engines().GetEngineByNameAndVariant(engine_name, variant);
}

std::optional<std::string> DatabaseService::DeleteEngineById(int id) {
  return cortex::db::Engines().DeleteEngineById(id);
}