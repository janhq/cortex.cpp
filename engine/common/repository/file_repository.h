#pragma once

#include "common/file.h"
#include "utils/result.hpp"

class FileRepository {
 public:
  virtual cpp::result<void, std::string> StoreFile(OpenAi::File& file_metadata,
                                                   const char* content,
                                                   uint64_t length) = 0;

  virtual cpp::result<std::vector<OpenAi::File>, std::string> ListFiles(
      const std::string& purpose, uint8_t limit, const std::string& order,
      const std::string& after) const = 0;

  virtual cpp::result<OpenAi::File, std::string> RetrieveFile(
      const std::string file_id) const = 0;

  virtual cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
  RetrieveFileContent(const std::string& file_id) const = 0;

  virtual cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
  RetrieveFileContentByPath(const std::string& path) const = 0;

  virtual cpp::result<void, std::string> DeleteFileLocal(
      const std::string& file_id) = 0;

  virtual ~FileRepository() = default;
};
