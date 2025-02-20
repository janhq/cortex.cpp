#include "file_service.h"
#include <cstdint>
#include "utils/ulid_generator.h"

cpp::result<OpenAi::File, std::string> FileService::UploadFile(
    const std::string& filename, const std::string& purpose,
    const char* content, uint64_t content_length) {

  auto seconds_since_epoch =
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();

  auto file_id{"file-" + ulid::GenerateUlid()};
  OpenAi::File file;
  file.id = file_id;
  file.object = "file";
  file.bytes = content_length;
  file.created_at = seconds_since_epoch;
  file.filename = filename;
  file.purpose = purpose;

  auto res = file_repository_->StoreFile(file, content, content_length);
  if (res.has_error()) {
    return cpp::fail(res.error());
  }

  return file;
}

cpp::result<std::vector<OpenAi::File>, std::string> FileService::ListFiles(
    const std::string& purpose, uint8_t limit, const std::string& order,
    const std::string& after) const {
  return file_repository_->ListFiles(purpose, limit, order, after);
}

cpp::result<OpenAi::File, std::string> FileService::RetrieveFile(
    const std::string& file_id) const {
  return file_repository_->RetrieveFile(file_id);
}

cpp::result<void, std::string> FileService::DeleteFileLocal(
    const std::string& file_id) {
  return file_repository_->DeleteFileLocal(file_id);
}

cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
FileService::RetrieveFileContent(const std::string& file_id) const {
  return file_repository_->RetrieveFileContent(file_id);
}

cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
FileService::RetrieveFileContentByPath(const std::string& path) const {
  return file_repository_->RetrieveFileContentByPath(path);
}
