#include "modellist_utils.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "file_manager_utils.h"

namespace modellist_utils {
const std::string ModelListUtils::kModelListPath =
    (file_manager_utils::GetModelsContainerPath() /
     std::filesystem::path("model.list"))
        .string();
namespace {
const std::string kDefaultDbPath =
    file_manager_utils::GetCortexDataPath().string() + "/cortex.db";
}
ModelListUtils::ModelListUtils()
    : db_(kDefaultDbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
  db_.exec(
      "CREATE TABLE IF NOT EXISTS models ("
      "model_id TEXT PRIMARY KEY,"
      "author_repo_id TEXT,"
      "branch_name TEXT,"
      "path_to_model_yaml TEXT,"
      "model_alias TEXT,"
      "status TEXT);");
}
ModelListUtils::~ModelListUtils() {}

std::vector<ModelEntry> ModelListUtils::LoadModelList() const {
  std::vector<ModelEntry> entries;
  SQLite::Statement query(
      db_,
      "SELECT model_id, author_repo_id, branch_name, "
      "path_to_model_yaml, model_alias, status FROM models");

  while (query.executeStep()) {
    ModelEntry entry;
    entry.model_id = query.getColumn(0).getString();
    entry.author_repo_id = query.getColumn(1).getString();
    entry.branch_name = query.getColumn(2).getString();
    entry.path_to_model_yaml = query.getColumn(3).getString();
    entry.model_alias = query.getColumn(4).getString();
    std::string status_str = query.getColumn(5).getString();
    entry.status =
        (status_str == "RUNNING") ? ModelStatus::RUNNING : ModelStatus::READY;
    entries.push_back(entry);
  }
  return entries;
}

bool ModelListUtils::IsUnique(const std::string& model_id,
                              const std::string& model_alias) const {
  SQLite::Statement query(db_,
                          "SELECT COUNT(*) FROM models WHERE model_id = ? OR "
                          "model_id = ? OR model_alias = ? OR model_alias = ?");
  query.bind(1, model_id);
  query.bind(2, model_alias);
  query.bind(3, model_id);
  query.bind(4, model_alias);
  if (query.executeStep()) {
    return query.getColumn(0).getInt() == 0;
  }
  return false;
}

void ModelListUtils::SaveModelList(
    const std::vector<ModelEntry>& entries) const {
  std::ofstream file(kModelListPath);
  db_.exec("BEGIN TRANSACTION;");
  for (const auto& entry : entries) {
    SQLite::Statement insert(
        db_,
        "INSERT OR REPLACE INTO models (model_id, author_repo_id, "
        "branch_name, path_to_model_yaml, model_alias, status) VALUES (?, ?, "
        "?, ?, ?, ?)");
    insert.bind(1, entry.model_id);
    insert.bind(2, entry.author_repo_id);
    insert.bind(3, entry.branch_name);
    insert.bind(4, entry.path_to_model_yaml);
    insert.bind(5, entry.model_alias);
    insert.bind(6,
                (entry.status == ModelStatus::RUNNING ? "RUNNING" : "READY"));
    insert.exec();
  }
  db_.exec("COMMIT;");
}

std::string ModelListUtils::GenerateShortenedAlias(
    const std::string& model_id) const {
  std::vector<std::string> parts;
  std::istringstream iss(model_id);
  std::string part;
  while (std::getline(iss, part, '/')) {
    parts.push_back(part);
  }

  if (parts.empty()) {
    return model_id;  // Return original if no parts
  }

  // Extract the filename without extension
  std::string filename = parts.back();
  size_t last_dot_pos = filename.find_last_of('.');
  if (last_dot_pos != std::string::npos) {
    filename = filename.substr(0, last_dot_pos);
  }

  // Convert to lowercase
  std::transform(filename.begin(), filename.end(), filename.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  // Generate alias candidates
  std::vector<std::string> candidates;
  candidates.push_back(filename);

  if (parts.size() >= 2) {
    candidates.push_back(parts[parts.size() - 2] + ":" + filename);
  }

  if (parts.size() >= 3) {
    candidates.push_back(parts[parts.size() - 3] + ":" +
                         parts[parts.size() - 2] + "/" + filename);
  }

  if (parts.size() >= 4) {
    candidates.push_back(parts[0] + ":" + parts[1] + "/" +
                         parts[parts.size() - 2] + "/" + filename);
  }

  // Find the first unique candidate
  for (const auto& candidate : candidates) {
    if (IsUnique(model_id, candidate)) {
      return candidate;
    }
  }

  // If all candidates are taken, append a number to the last candidate
  std::string base_candidate = candidates.back();
  int suffix = 1;
  std::string unique_candidate = base_candidate;
  while (!IsUnique(model_id, unique_candidate)) {
    unique_candidate = base_candidate + "-" + std::to_string(suffix++);
  }

  return unique_candidate;
}

ModelEntry ModelListUtils::GetModelInfo(const std::string& identifier) const {
  std::lock_guard<std::mutex> lock(mutex_);
  SQLite::Statement query(db_,
                          "SELECT model_id, author_repo_id, branch_name, "
                          "path_to_model_yaml, model_alias, status FROM models "
                          "WHERE model_id = ? OR model_alias = ?");

  query.bind(1, identifier);
  query.bind(2, identifier);
  if (query.executeStep()) {
    ModelEntry entry;
    entry.model_id = query.getColumn(0).getString();
    entry.author_repo_id = query.getColumn(1).getString();
    entry.branch_name = query.getColumn(2).getString();
    entry.path_to_model_yaml = query.getColumn(3).getString();
    entry.model_alias = query.getColumn(4).getString();
    std::string status_str = query.getColumn(5).getString();
    entry.status =
        (status_str == "RUNNING") ? ModelStatus::RUNNING : ModelStatus::READY;
    return entry;
  } else {
    throw std::runtime_error("Model not found: " + identifier);
  }
}

void ModelListUtils::PrintModelInfo(const ModelEntry& entry) const {
  LOG_INFO << "Model ID: " << entry.model_id;
  LOG_INFO << "Author/Repo ID: " << entry.author_repo_id;
  LOG_INFO << "Branch Name: " << entry.branch_name;
  LOG_INFO << "Path to model.yaml: " << entry.path_to_model_yaml;
  LOG_INFO << "Model Alias: " << entry.model_alias;
  LOG_INFO << "Status: "
           << (entry.status == ModelStatus::RUNNING ? "RUNNING" : "READY");
}

bool ModelListUtils::AddModelEntry(ModelEntry new_entry, bool use_short_alias) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (IsUnique(new_entry.model_id, new_entry.model_alias)) {
    if (use_short_alias) {
      new_entry.model_alias = GenerateShortenedAlias(new_entry.model_id);
    }
    new_entry.status = ModelStatus::READY;  // Set default status to READY

    SQLite::Statement insert(
        db_,
        "INSERT INTO models (model_id, author_repo_id, "
        "branch_name, path_to_model_yaml, model_alias, status) VALUES (?, ?, "
        "?, ?, ?, ?)");
    insert.bind(1, new_entry.model_id);
    insert.bind(2, new_entry.author_repo_id);
    insert.bind(3, new_entry.branch_name);
    insert.bind(4, new_entry.path_to_model_yaml);
    insert.bind(5, new_entry.model_alias);
    insert.bind(
        6, (new_entry.status == ModelStatus::RUNNING ? "RUNNING" : "READY"));
    insert.exec();

    return true;
  }
  return false;  // Entry not added due to non-uniqueness
}

bool ModelListUtils::UpdateModelEntry(const std::string& identifier,
                                      const ModelEntry& updated_entry) {
  std::lock_guard<std::mutex> lock(mutex_);
  SQLite::Statement upd(db_,
                        "UPDATE models "
                        "SET author_repo_id = ?, branch_name = ?, "
                        "path_to_model_yaml = ?, status = ? "
                        "WHERE model_id = ? OR model_alias = ?");
  upd.bind(1, updated_entry.author_repo_id);
  upd.bind(2, updated_entry.branch_name);
  upd.bind(3, updated_entry.path_to_model_yaml);
  upd.bind(
      4, (updated_entry.status == ModelStatus::RUNNING ? "RUNNING" : "READY"));
  upd.bind(5, identifier);
  upd.bind(6, identifier);
  return upd.exec() == 1;
}

bool ModelListUtils::UpdateModelAlias(const std::string& model_id,
                                      const std::string& new_model_alias) {
  std::lock_guard<std::mutex> lock(mutex_);
  SQLite::Statement upd(db_,
                        "UPDATE models "
                        "SET model_alias = ? "
                        "WHERE model_id = ? OR model_alias = ?");
  upd.bind(1, new_model_alias);
  upd.bind(2, model_id);
  upd.bind(3, model_id);
  return upd.exec() == 1;
}

bool ModelListUtils::DeleteModelEntry(const std::string& identifier) {
  std::lock_guard<std::mutex> lock(mutex_);
  SQLite::Statement del(
      db_, "DELETE from models WHERE model_id = ? OR model_alias = ?");
  del.bind(1, identifier);
  del.bind(2, identifier);
  return del.exec() == 1;
}

bool ModelListUtils::HasModel(const std::string& identifier) const {
  std::lock_guard<std::mutex> lock(mutex_);
  SQLite::Statement query(
      db_, "SELECT COUNT(*) FROM models WHERE model_id = ? OR model_alias = ?");
  query.bind(1, identifier);
  query.bind(2, identifier);
  if (query.executeStep()) {
    return query.getColumn(0).getInt() > 0;
  }
  return false;
}
}  // namespace modellist_utils
