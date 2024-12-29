#pragma once
#include "database/engines.h"
#include "database/file.h"
#include "database/hardware.h"
#include "database/models.h"

using EngineEntry = cortex::db::EngineEntry;
class DatabaseService {
 public:
  // engines
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

  // file
  cpp::result<std::vector<OpenAi::File>, std::string> GetFileList() const;

  cpp::result<OpenAi::File, std::string> GetFileById(
      const std::string& file_id) const;

  cpp::result<void, std::string> AddFileEntry(OpenAi::File& file);

  cpp::result<void, std::string> DeleteFileEntry(const std::string& file_id);

  // hardware

  // models
 private:
};