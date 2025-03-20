#include "file_fs_repository.h"
#include <json/reader.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include "database/file.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"

std::filesystem::path FileFsRepository::GetFilePath() const {
  return data_folder_path_ / kFileContainerFolderName;
}

std::filesystem::path SanitizePath(const std::filesystem::path& user_input,
                                   const std::filesystem::path& basedir) {

  auto abs_base = std::filesystem::canonical(basedir);
  std::filesystem::path resolved_path = std::filesystem::weakly_canonical(
      std::filesystem::path(basedir) / std::filesystem::path(user_input));
  /* Ensure the resolved path is within our basedir */
  for (auto p = resolved_path; !p.empty(); p = p.parent_path()) {
    if (std::filesystem::equivalent(p, abs_base)) {
      return resolved_path;
    }
  }
  return {};
}

cpp::result<void, std::string> FileFsRepository::StoreFile(
    OpenAi::File& file_metadata, const char* content, uint64_t length) {
  auto file_container_path = GetFilePath();
  if (!std::filesystem::exists(file_container_path)) {
    std::filesystem::create_directories(file_container_path);
  }

  auto original_filename = file_metadata.filename;
  auto file_full_path = SanitizePath(original_filename, file_container_path);

  if (file_full_path.empty()) {
    return cpp::fail("Error resolving path in: " + original_filename);
  }

  // Handle duplicate filenames
  int counter = 1;
  while (std::filesystem::exists(file_full_path)) {
    auto dot_pos = original_filename.find_last_of('.');
    std::string name_part;
    std::string ext_part;

    if (dot_pos != std::string::npos) {
      name_part = original_filename.substr(0, dot_pos);
      ext_part = original_filename.substr(dot_pos);
    } else {
      name_part = original_filename;
      ext_part = "";
    }

    auto new_filename = name_part + "_" + std::to_string(counter) + ext_part;
    file_full_path = file_container_path / new_filename;
    file_metadata.filename = new_filename;
    counter++;
  }

  try {
    std::ofstream file(file_full_path, std::ios::binary);
    if (!file) {
      return cpp::fail("Failed to open file for writing: " +
                       file_full_path.string());
    }

    file.write(content, length);
    file.flush();
    file.close();

    auto result = db_service_->AddFileEntry(file_metadata);
    if (result.has_error()) {
      std::filesystem::remove(file_full_path);
      return cpp::fail(result.error());
    }

    return {};
  } catch (const std::exception& e) {
    CTL_ERR("Failed to store file: " << e.what());
    return cpp::fail("Failed to write file: " + file_full_path.string() +
                     ", error: " + e.what());
  }
}

cpp::result<std::vector<OpenAi::File>, std::string> FileFsRepository::ListFiles(
    const std::string& purpose, uint8_t limit, const std::string& order,
    const std::string& after) const {
  (void)purpose;
  (void)after;
  auto res = db_service_->GetFileList();
  if (res.has_error()) {
    return cpp::fail(res.error());
  }
  auto files = res.value();

  if (order == "desc") {
    std::sort(files.begin(), files.end(),
              [](const OpenAi::File& a, const OpenAi::File& b) {
                return a.id > b.id;
              });
  } else {
    std::sort(files.begin(), files.end(),
              [](const OpenAi::File& a, const OpenAi::File& b) {
                return a.id < b.id;
              });
  }

  if (limit > 0 && files.size() > limit) {
    files.resize(limit);
  }

  return files;
}

cpp::result<OpenAi::File, std::string> FileFsRepository::RetrieveFile(
    const std::string file_id) const {
  CTL_INF("Retrieving file: " + file_id);

  auto file_container_path = GetFilePath();
  auto res = db_service_->GetFileById(file_id);
  if (res.has_error()) {
    return cpp::fail(res.error());
  }

  return res.value();
}

cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
FileFsRepository::RetrieveFileContent(const std::string& file_id) const {
  auto file_container_path = GetFilePath();
  auto file_metadata = RetrieveFile(file_id);
  if (file_metadata.has_error()) {
    return cpp::fail(file_metadata.error());
  }
  auto file_path = file_container_path / file_metadata->filename;
  if (!std::filesystem::exists(file_path)) {
    return cpp::fail("File content not found: " + file_path.string());
  }
  size_t size = std::filesystem::file_size(file_path);
  auto buffer = std::make_unique<char[]>(size);
  std::ifstream file(file_path, std::ios::binary);
  if (!file.read(buffer.get(), size)) {
    return cpp::fail("Failed to read file: " + file_path.string());
  }

  return std::make_pair(std::move(buffer), size);
}

cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
FileFsRepository::RetrieveFileContentByPath(const std::string& path) const {
  auto file_path = data_folder_path_ / path;
  if (!std::filesystem::exists(file_path)) {
    return cpp::fail("File not found: " + path);
  }

  try {
    size_t size = std::filesystem::file_size(file_path);
    auto buffer = std::make_unique<char[]>(size);

    std::ifstream file(file_path, std::ios::binary);
    if (!file.read(buffer.get(), size)) {
      return cpp::fail("Failed to read file: " + file_path.string());
    }

    return std::make_pair(std::move(buffer), size);
  } catch (const std::exception& e) {
    CTL_ERR("Failed to retrieve file content: " << e.what());
    return cpp::fail("Failed to retrieve file content");
  }
}

cpp::result<void, std::string> FileFsRepository::DeleteFileLocal(
    const std::string& file_id) {
  CTL_INF("Deleting file: " + file_id);
  auto file_container_path = GetFilePath();
  auto file_metadata = db_service_->GetFileById(file_id);
  if (file_metadata.has_error()) {
    return cpp::fail(file_metadata.error());
  }

  auto file_path = file_container_path / file_metadata->filename;

  auto res = db_service_->DeleteFileEntry(file_id);
  if (res.has_error()) {
    CTL_ERR("Failed to delete file entry: " << res.error());
    return cpp::fail(res.error());
  }

  if (!std::filesystem::exists(file_path)) {
    CTL_INF("File not found: " + file_path.string());
    return {};
  }

  try {
    std::filesystem::remove_all(file_path);
    return {};
  } catch (const std::exception& e) {
    CTL_ERR("Failed to delete file: " << e.what());
    return cpp::fail("Failed to delete file: " + file_container_path.string() +
                     ", error: " + e.what());
  }
}
