#include "modellist_utils.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include "file_manager_utils.h"
namespace modellist_utils {
const std::string ModelListUtils::kModelListPath =
    (file_manager_utils::GetModelsContainerPath() /
     std::filesystem::path("model.list"))
        .string();

std::vector<ModelEntry> ModelListUtils::LoadModelList() const {
  std::vector<ModelEntry> entries;
  std::filesystem::path file_path(kModelListPath);

  // Check if the file exists, if not, create it
  if (!std::filesystem::exists(file_path)) {
    std::ofstream create_file(kModelListPath);
    if (!create_file) {
      throw std::runtime_error("Unable to create model.list file: " +
                               kModelListPath);
    }
    create_file.close();
    return entries;  // Return empty vector for newly created file
  }

  std::ifstream file(kModelListPath);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open model.list file: " +
                             kModelListPath);
  }

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    ModelEntry entry;
    std::string status_str;
    if (!(iss >> entry.model_id >> entry.author_repo_id >> entry.branch_name >>
          entry.path_to_model_yaml >> entry.model_alias >> status_str)) {
      LOG_WARN << "Invalid entry in model.list: " << line;
    } else {
      entry.status =
          (status_str == "RUNNING") ? ModelStatus::RUNNING : ModelStatus::READY;
      entries.push_back(entry);
    }
  }
  return entries;
}

bool ModelListUtils::IsUnique(const std::vector<ModelEntry>& entries,
                              const std::string& model_id,
                              const std::string& model_alias) const {
  return std::none_of(
      entries.begin(), entries.end(), [&](const ModelEntry& entry) {
        return entry.model_id == model_id || entry.model_alias == model_id ||
               entry.model_id == model_alias ||
               entry.model_alias == model_alias;
      });
}

void ModelListUtils::SaveModelList(
    const std::vector<ModelEntry>& entries) const {
  std::ofstream file(kModelListPath);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open model.list file for writing: " +
                             kModelListPath);
  }

  for (const auto& entry : entries) {
    file << entry.model_id << " " << entry.author_repo_id << " "
         << entry.branch_name << " " << entry.path_to_model_yaml << " "
         << entry.model_alias << " "
         << (entry.status == ModelStatus::RUNNING ? "RUNNING" : "READY")
         << std::endl;
  }
}

std::string ModelListUtils::GenerateShortenedAlias(
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

ModelEntry ModelListUtils::GetModelInfo(const std::string& identifier) const {
  std::lock_guard<std::mutex> lock(mutex_);
  auto entries = LoadModelList();
  auto it = std::find_if(
      entries.begin(), entries.end(), [&identifier](const ModelEntry& entry) {
        return entry.model_id == identifier || entry.model_alias == identifier;
      });

  if (it != entries.end()) {
    return *it;
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
  auto entries = LoadModelList();

  if (IsUnique(entries, new_entry.model_id, new_entry.model_alias)) {
    if (use_short_alias) {
      new_entry.model_alias =
          GenerateShortenedAlias(new_entry.model_id, entries);
    }
    new_entry.status = ModelStatus::READY;  // Set default status to READY
    entries.push_back(std::move(new_entry));
    SaveModelList(entries);
    return true;
  }
  return false;  // Entry not added due to non-uniqueness
}

bool ModelListUtils::UpdateModelEntry(const std::string& identifier,
                                      const ModelEntry& updated_entry) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto entries = LoadModelList();
  auto it = std::find_if(
      entries.begin(), entries.end(), [&identifier](const ModelEntry& entry) {
        return entry.model_id == identifier || entry.model_alias == identifier;
      });

  if (it != entries.end()) {
    *it = updated_entry;
    SaveModelList(entries);
    return true;
  }
  return false;  // Entry not found
}

bool ModelListUtils::DeleteModelEntry(const std::string& identifier) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto entries = LoadModelList();
  auto it = std::find_if(entries.begin(), entries.end(),
                         [&identifier](const ModelEntry& entry) {
                           return (entry.model_id == identifier ||
                                   entry.model_alias == identifier) &&
                                  entry.status == ModelStatus::READY;
                         });

  if (it != entries.end()) {
    entries.erase(it);
    SaveModelList(entries);
    return true;
  }
  return false;  // Entry not found or not in READY state
}
}  // namespace modellist_utils