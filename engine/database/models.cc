#include "models.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "database.h"
#include "utils/file_manager_utils.h"
#include "utils/result.hpp"
#include "utils/scope_exit.h"

namespace cortex::db {

Models::Models() : db_(cortex::db::Database::GetInstance().db()) {
  db_.exec(
      "CREATE TABLE IF NOT EXISTS models ("
      "model_id TEXT PRIMARY KEY,"
      "author_repo_id TEXT,"
      "branch_name TEXT,"
      "path_to_model_yaml TEXT,"
      "model_alias TEXT);");
}

Models::Models(SQLite::Database& db) : db_(db) {
  db_.exec(
      "CREATE TABLE IF NOT EXISTS models ("
      "model_id TEXT PRIMARY KEY,"
      "author_repo_id TEXT,"
      "branch_name TEXT,"
      "path_to_model_yaml TEXT,"
      "model_alias TEXT UNIQUE);");
}

Models::~Models() {}

cpp::result<std::vector<ModelEntry>, std::string> Models::LoadModelList()
    const {
  try {
    db_.exec("BEGIN TRANSACTION;");
    utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    return LoadModelListNoLock();
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

bool Models::IsUnique(const std::vector<ModelEntry>& entries,
                      const std::string& model_id,
                      const std::string& model_alias) const {
  return std::none_of(
      entries.begin(), entries.end(), [&](const ModelEntry& entry) {
        return entry.model == model_id || entry.model_alias == model_id ||
               entry.model == model_alias ||
               entry.model_alias == model_alias;
      });
}

cpp::result<std::vector<ModelEntry>, std::string> Models::LoadModelListNoLock()
    const {
  try {
    std::vector<ModelEntry> entries;
    SQLite::Statement query(db_,
                            "SELECT model_id, author_repo_id, branch_name, "
                            "path_to_model_yaml, model_alias FROM models");

    while (query.executeStep()) {
      ModelEntry entry;
      entry.model = query.getColumn(0).getString();
      entry.author_repo_id = query.getColumn(1).getString();
      entry.branch_name = query.getColumn(2).getString();
      entry.path_to_model_yaml = query.getColumn(3).getString();
      entry.model_alias = query.getColumn(4).getString();
      entries.push_back(entry);
    }
    return entries;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

std::string Models::GenerateShortenedAlias(
    const std::string& model_id, const std::vector<ModelEntry>& entries) const {
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
    if (IsUnique(entries, model_id, candidate)) {
      return candidate;
    }
  }

  // If all candidates are taken, append a number to the last candidate
  std::string base_candidate = candidates.back();
  int suffix = 1;
  std::string unique_candidate = base_candidate;
  while (!IsUnique(entries, model_id, unique_candidate)) {
    unique_candidate = base_candidate + "-" + std::to_string(suffix++);
  }

  return unique_candidate;
}

cpp::result<ModelEntry, std::string> Models::GetModelInfo(
    const std::string& identifier) const {
  try {
    SQLite::Statement query(db_,
                            "SELECT model_id, author_repo_id, branch_name, "
                            "path_to_model_yaml, model_alias FROM models "
                            "WHERE model_id = ? OR model_alias = ?");

    query.bind(1, identifier);
    query.bind(2, identifier);
    if (query.executeStep()) {
      ModelEntry entry;
      entry.model = query.getColumn(0).getString();
      entry.author_repo_id = query.getColumn(1).getString();
      entry.branch_name = query.getColumn(2).getString();
      entry.path_to_model_yaml = query.getColumn(3).getString();
      entry.model_alias = query.getColumn(4).getString();
      return entry;
    } else {
      return cpp::fail("Model not found: " + identifier);
    }
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

void Models::PrintModelInfo(const ModelEntry& entry) const {
  LOG_INFO << "Model ID: " << entry.model;
  LOG_INFO << "Author/Repo ID: " << entry.author_repo_id;
  LOG_INFO << "Branch Name: " << entry.branch_name;
  LOG_INFO << "Path to model.yaml: " << entry.path_to_model_yaml;
  LOG_INFO << "Model Alias: " << entry.model_alias;
}

cpp::result<bool, std::string> Models::AddModelEntry(ModelEntry new_entry,
                                                     bool use_short_alias) {
  try {
    db_.exec("BEGIN TRANSACTION;");
    utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    auto model_list = LoadModelListNoLock();
    if (model_list.has_error()) {
      CTL_WRN(model_list.error());
      std::cout << "Test: " << model_list.error();
      return cpp::fail(model_list.error());
    }
    if (IsUnique(model_list.value(), new_entry.model,
                 new_entry.model_alias)) {
      if (use_short_alias) {
        new_entry.model_alias =
            GenerateShortenedAlias(new_entry.model, model_list.value());
      }

      SQLite::Statement insert(
          db_,
          "INSERT INTO models (model_id, author_repo_id, "
          "branch_name, path_to_model_yaml, model_alias) VALUES (?, ?, "
          "?, ?, ?)");
      insert.bind(1, new_entry.model);
      insert.bind(2, new_entry.author_repo_id);
      insert.bind(3, new_entry.branch_name);
      insert.bind(4, new_entry.path_to_model_yaml);
      insert.bind(5, new_entry.model_alias);
      insert.exec();

      return true;
    }
    return false;  // Entry not added due to non-uniqueness
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

cpp::result<bool, std::string> Models::UpdateModelEntry(
    const std::string& identifier, const ModelEntry& updated_entry) {
  if (!HasModel(identifier)) {
    return cpp::fail("Model not found: " + identifier);
  }
  try {
    SQLite::Statement upd(db_,
                          "UPDATE models "
                          "SET author_repo_id = ?, branch_name = ?, "
                          "path_to_model_yaml = ? "
                          "WHERE model_id = ? OR model_alias = ?");
    upd.bind(1, updated_entry.author_repo_id);
    upd.bind(2, updated_entry.branch_name);
    upd.bind(3, updated_entry.path_to_model_yaml);
    upd.bind(4, identifier);
    upd.bind(5, identifier);
    return upd.exec() == 1;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

cpp::result<bool, std::string> Models::UpdateModelAlias(
    const std::string& model_id, const std::string& new_model_alias) {
  if (!HasModel(model_id)) {
    return cpp::fail("Model not found: " + model_id);
  }
  try {
    db_.exec("BEGIN TRANSACTION;");
    utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    auto model_list = LoadModelListNoLock();
    if (model_list.has_error()) {
      CTL_WRN(model_list.error());
      return cpp::fail(model_list.error());
    }
    // Check new_model_alias is unique
    if (IsUnique(model_list.value(), new_model_alias, new_model_alias)) {
      SQLite::Statement upd(db_,
                            "UPDATE models "
                            "SET model_alias = ? "
                            "WHERE model_id = ? OR model_alias = ?");
      upd.bind(1, new_model_alias);
      upd.bind(2, model_id);
      upd.bind(3, model_id);
      return upd.exec() == 1;
    }
    return false;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

cpp::result<bool, std::string> Models::DeleteModelEntry(
    const std::string& identifier) {
  try {
    SQLite::Statement del(
        db_, "DELETE from models WHERE model_id = ? OR model_alias = ?");
    del.bind(1, identifier);
    del.bind(2, identifier);
    return del.exec() == 1;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

bool Models::HasModel(const std::string& identifier) const {
  try {
    SQLite::Statement query(
        db_,
        "SELECT COUNT(*) FROM models WHERE model_id = ? OR model_alias = ?");
    query.bind(1, identifier);
    query.bind(2, identifier);
    if (query.executeStep()) {
      return query.getColumn(0).getInt() > 0;
    }
    return false;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return false;
  }
}
}  // namespace cortex::db
