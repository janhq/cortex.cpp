#pragma once

#include "common/file.h"
#include "common/repository/file_repository.h"
#include "utils/result.hpp"

class FileService {
 public:
  const std::vector<std::string> kSupportedPurposes{"assistants", "vision",
                                                    "batch", "fine-tune"};

  cpp::result<OpenAi::File, std::string> UploadFile(const std::string& filename,
                                                    const std::string& purpose,
                                                    const char* content,
                                                    uint64_t content_length);

  cpp::result<std::vector<OpenAi::File>, std::string> ListFiles(
      const std::string& purpose, uint8_t limit, const std::string& order,
      const std::string& after) const;

  cpp::result<OpenAi::File, std::string> RetrieveFile(
      const std::string& file_id) const;

  cpp::result<void, std::string> DeleteFileLocal(const std::string& file_id);

  cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
  RetrieveFileContent(const std::string& file_id) const;

  /**
   * For getting file content by **relative** path.
   */
  cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
  RetrieveFileContentByPath(const std::string& path) const;

  explicit FileService(std::shared_ptr<FileRepository> file_repository)
      : file_repository_{file_repository} {}

 private:
  std::shared_ptr<FileRepository> file_repository_;
};
