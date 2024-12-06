#include "models.h"
#include <algorithm>
#include <sstream>
#include "database.h"
#include "utils/result.hpp"
#include "utils/scope_exit.h"

namespace cortex::db {

Models::Models() : db_(cortex::db::Database::GetInstance().db()) {}

Models::~Models() {}

std::string Models::StatusToString(ModelStatus status) const {
  switch (status) {
    case ModelStatus::Remote:
      return "remote";
    case ModelStatus::Downloaded:
      return "downloaded";
    case ModelStatus::Undownloaded:
      return "undownloaded";
  }
  return "unknown";
}

Models::Models(SQLite::Database& db) : db_(db) {}

ModelStatus Models::StringToStatus(const std::string& status_str) const {
  if (status_str == "remote") {
    return ModelStatus::Remote;
  } else if (status_str == "downloaded" || status_str.empty()) {
    return ModelStatus::Downloaded;
  } else if (status_str == "undownloaded") {
    return ModelStatus::Undownloaded;
  }
  throw std::invalid_argument("Invalid status string");
}

cpp::result<std::vector<ModelEntry>, std::string> Models::LoadModelList()
    const {
  try {
    db_.exec("BEGIN TRANSACTION;");
    cortex::utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    return LoadModelListNoLock();
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

bool Models::IsUnique(const std::vector<ModelEntry>& entries,
                      const std::string& model_id) const {
  return std::none_of(
      entries.begin(), entries.end(),
      [&](const ModelEntry& entry) { return entry.model == model_id; });
}

cpp::result<std::vector<ModelEntry>, std::string> Models::LoadModelListNoLock()
    const {
  try {
    std::vector<ModelEntry> entries;
    SQLite::Statement query(db_,
                            "SELECT model_id, author_repo_id, branch_name, "
                            "path_to_model_yaml, model_alias, model_format, "
                            "model_source, status, engine FROM models");

    while (query.executeStep()) {
      ModelEntry entry;
      entry.model = query.getColumn(0).getString();
      entry.author_repo_id = query.getColumn(1).getString();
      entry.branch_name = query.getColumn(2).getString();
      entry.path_to_model_yaml = query.getColumn(3).getString();
      entry.model_alias = query.getColumn(4).getString();
      entry.model_format = query.getColumn(5).getString();
      entry.model_source = query.getColumn(6).getString();
      entry.status = StringToStatus(query.getColumn(7).getString());
      entry.engine = query.getColumn(8).getString();
      entries.push_back(entry);
    }
    return entries;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

cpp::result<ModelEntry, std::string> Models::GetModelInfo(
    const std::string& identifier) const {
  try {
    SQLite::Statement query(db_,
                            "SELECT model_id, author_repo_id, branch_name, "
                            "path_to_model_yaml, model_alias, model_format, "
                            "model_source, status, engine FROM models "
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
      entry.model_format = query.getColumn(5).getString();
      entry.model_source = query.getColumn(6).getString();
      entry.status = StringToStatus(query.getColumn(7).getString());
      entry.engine = query.getColumn(8).getString();
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
  LOG_INFO << "Model Format: " << entry.model_format;
  LOG_INFO << "Model Source: " << entry.model_source;
  LOG_INFO << "Status: " << StatusToString(entry.status);
  LOG_INFO << "Engine: " << entry.engine;
}

cpp::result<bool, std::string> Models::AddModelEntry(ModelEntry new_entry) {
  try {
    db_.exec("BEGIN TRANSACTION;");
    cortex::utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    auto model_list = LoadModelListNoLock();
    if (model_list.has_error()) {
      CTL_WRN(model_list.error());
      return cpp::fail(model_list.error());
    }
    if (IsUnique(model_list.value(), new_entry.model)) {

      SQLite::Statement insert(
          db_,
          "INSERT INTO models (model_id, author_repo_id, branch_name, "
          "path_to_model_yaml, model_alias, model_format, model_source, "
          "status, engine) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
      insert.bind(1, new_entry.model);
      insert.bind(2, new_entry.author_repo_id);
      insert.bind(3, new_entry.branch_name);
      insert.bind(4, new_entry.path_to_model_yaml);
      insert.bind(5, new_entry.model_alias);
      insert.bind(6, new_entry.model_format);
      insert.bind(7, new_entry.model_source);
      insert.bind(8, StatusToString(new_entry.status));
      insert.bind(9, new_entry.engine);
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
    SQLite::Statement upd(
        db_,
        "UPDATE models SET author_repo_id = ?, branch_name = ?, "
        "path_to_model_yaml = ?, model_format = ?, model_source = ?, status = "
        "?, engine = ? WHERE model_id = ? OR model_alias = ?");
    upd.bind(1, updated_entry.author_repo_id);
    upd.bind(2, updated_entry.branch_name);
    upd.bind(3, updated_entry.path_to_model_yaml);
    upd.bind(4, updated_entry.model_format);
    upd.bind(5, updated_entry.model_source);
    upd.bind(6, StatusToString(updated_entry.status));
    upd.bind(7, updated_entry.engine);
    upd.bind(8, identifier);
    upd.bind(9, identifier);
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
    cortex::utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    auto model_list = LoadModelListNoLock();
    if (model_list.has_error()) {
      CTL_WRN(model_list.error());
      return cpp::fail(model_list.error());
    }
    // Check new_model_alias is unique
    if (IsUnique(model_list.value(), new_model_alias)) {
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
    // delete only if its there
    if (!HasModel(identifier)) {
      return true;
    }

    SQLite::Statement del(
        db_, "DELETE from models WHERE model_id = ? OR model_alias = ?");
    del.bind(1, identifier);
    del.bind(2, identifier);
    return del.exec() == 1;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

cpp::result<bool, std::string> Models::DeleteModelEntryWithOrg(
    const std::string& src) {
  try {
    SQLite::Statement del(db_,
                          "DELETE from models WHERE model_source LIKE ? AND "
                          "status = \"undownloaded\"");
    del.bind(1, src + "%");
    return del.exec() == 1;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

cpp::result<bool, std::string> Models::DeleteModelEntryWithRepo(
    const std::string& src) {
  try {
    SQLite::Statement del(db_,
                          "DELETE from models WHERE model_source = ? AND "
                          "status = \"undownloaded\"");
    del.bind(1, src);
    return del.exec() == 1;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

cpp::result<std::vector<std::string>, std::string> Models::FindRelatedModel(
    const std::string& identifier) const {
  try {
    std::vector<std::string> related_models;
    SQLite::Statement query(
        db_, "SELECT model_id FROM models WHERE model_id LIKE ?");
    query.bind(1, "%" + identifier + "%");

    while (query.executeStep()) {
      related_models.push_back(query.getColumn(0).getString());
    }
    return related_models;
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

cpp::result<std::vector<std::string>, std::string> Models::GetModelSources()
    const {
  try {
    std::vector<std::string> sources;
    SQLite::Statement query(db_,
                            "SELECT DISTINCT model_source FROM models WHERE "
                            "status = \"undownloaded\"");

    while (query.executeStep()) {
      sources.push_back(query.getColumn(0).getString());
    }
    return sources;
  } catch (const std::exception& e) {
    return cpp::fail(e.what());
  }
}

}  // namespace cortex::db