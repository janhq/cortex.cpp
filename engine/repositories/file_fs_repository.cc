#include "file_fs_repository.h"
#include <json/reader.h>
#include <filesystem>
#include <fstream>
#include "utils/logging_utils.h"
#include "utils/result.hpp"

std::filesystem::path FileFsRepository::GetFilePath(
    const std::string& file_id) const {
  return data_folder_path_ / kFileContainerFolderName / file_id;
}

cpp::result<void, std::string> FileFsRepository::StoreFile(
    OpenAi::File& file_metadata, const char* content, uint64_t length) {
  auto file_container_path = GetFilePath(file_metadata.id);
  if (!std::filesystem::exists(file_container_path)) {
    std::filesystem::create_directories(file_container_path);
  }

  auto file_full_path = file_container_path / file_metadata.filename;

  try {
    std::ofstream file(file_full_path, std::ios::binary);
    if (!file) {
      return cpp::fail("Failed to open file for writing: " +
                       file_full_path.string());
    }

    file.write(content, length);
    file.flush();
    file.close();

    // writing metadata file
    auto meta_file_full_path = file_container_path / kFileMetadataFileName;
    CTL_INF("Metadata path: " + meta_file_full_path.string());
    std::ofstream metadata_file(meta_file_full_path, std::ios::trunc);
    if (!metadata_file) {
      return cpp::fail("Failed to open file for writing: " +
                       meta_file_full_path.string());
    }

    metadata_file << file_metadata.ToJson().value().toStyledString();
    metadata_file.flush();
    metadata_file.close();

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
  std::lock_guard<std::mutex> lock(fs_mutex_);
  try {
    std::vector<OpenAi::File> files;
    auto files_path = data_folder_path_ / kFileContainerFolderName;

    for (const auto& entry : std::filesystem::directory_iterator(files_path)) {
      if (!entry.is_directory())
        continue;
      auto metadata_path = entry.path() / kFileMetadataFileName;
      if (!std::filesystem::exists(metadata_path))
        continue;

      auto read_metadata_result = ReadFileMetadata(metadata_path);
      if (read_metadata_result.has_error()) {
        CTL_WRN("Failed to read metadata: " + read_metadata_result.error());
        continue;
      }
      files.push_back(std::move(read_metadata_result.value()));
    }

    // Sort by ULID
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
  } catch (const std::exception& e) {
    CTL_ERR("Failed to list files: " << e.what());
    return cpp::fail("Failed to list files");
  }
}

cpp::result<OpenAi::File, std::string> FileFsRepository::ReadFileMetadata(
    const std::filesystem::path& file_path) const {

  std::ifstream metadata_file(file_path);
  if (!metadata_file) {
    CTL_ERR("Failed to open metadata file: " + file_path.string());
    return cpp::fail("Failed to open metadata file: " + file_path.string());
  }

  Json::Value root;
  Json::CharReaderBuilder builder;
  JSONCPP_STRING errs;

  if (!parseFromStream(builder, metadata_file, &root, &errs)) {
    return cpp::fail("Failed to parse JSON: " + errs);
  }

  try {
    auto metadata = OpenAi::File::FromJson(root);
    auto origin_file_path = file_path.parent_path() / metadata->filename;
    if (!std::filesystem::exists(origin_file_path)) {
      return cpp::fail("File not found: " + origin_file_path.string());
    }
    return metadata;
  } catch (const std::exception& e) {
    CTL_ERR("Failed to parse file metadata: " << e.what());
    return cpp::fail("Failed to parse file metadata");
  }
}

cpp::result<OpenAi::File, std::string> FileFsRepository::RetrieveFile(
    const std::string file_id) const {
  CTL_INF("Retrieving file: " + file_id);
  auto file_container_path = GetFilePath(file_id);
  auto metadata_file_path = file_container_path / kFileMetadataFileName;
  std::lock_guard<std::mutex> lock(fs_mutex_);

  if (!std::filesystem::exists(metadata_file_path)) {
    CTL_ERR("File not found: " + metadata_file_path.string());
    return cpp::fail("File not found: " + metadata_file_path.string());
  }

  return ReadFileMetadata(metadata_file_path);
}

cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
FileFsRepository::RetrieveFileContent(const std::string& file_id) const {
  auto file_container_path = GetFilePath(file_id);
  auto metadata_file_path = file_container_path / kFileMetadataFileName;
  std::lock_guard<std::mutex> lock(fs_mutex_);

  try {
    auto file_metadata_result = ReadFileMetadata(metadata_file_path);
    if (!file_metadata_result) {
      return cpp::fail(file_metadata_result.error());
    }

    auto file_path =
        GetFilePath(file_id) / file_metadata_result.value().filename;
    if (!std::filesystem::exists(file_path)) {
      return cpp::fail("File content not found: " + file_id);
    }

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

cpp::result<void, std::string> FileFsRepository::DeleteFile(
    const std::string& file_id) {
  CTL_INF("Deleting file: " + file_id);
  auto file_container_path = GetFilePath(file_id);

  std::lock_guard<std::mutex> lock(fs_mutex_);
  if (!std::filesystem::exists(file_container_path)) {
    CTL_INF("File not found: " + file_container_path.string());
    return {};
  }

  try {
    std::filesystem::remove_all(file_container_path);
    return {};
  } catch (const std::exception& e) {
    CTL_ERR("Failed to delete file: " << e.what());
    return cpp::fail("Failed to delete file: " + file_container_path.string() +
                     ", error: " + e.what());
  }
}
