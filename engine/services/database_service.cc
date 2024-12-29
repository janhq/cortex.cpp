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
// end engines

// file
cpp::result<std::vector<OpenAi::File>, std::string>
DatabaseService::GetFileList() const {
  return cortex::db::File().GetFileList();
}

cpp::result<OpenAi::File, std::string> DatabaseService::GetFileById(
    const std::string& file_id) const {
  return cortex::db::File().GetFileById(file_id);
}

cpp::result<void, std::string> DatabaseService::AddFileEntry(
    OpenAi::File& file) {
  return cortex::db::File().AddFileEntry(file);
}

cpp::result<void, std::string> DatabaseService::DeleteFileEntry(
    const std::string& file_id) {
  return cortex::db::File().DeleteFileEntry(file_id);
}
// end file