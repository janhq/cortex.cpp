#pragma once

#include <filesystem>
#include "common/repository/file_repository.h"
#include "services/database_service.h"
#include "utils/logging_utils.h"

class FileFsRepository : public FileRepository {
 public:
  constexpr static auto kFileContainerFolderName = "files";

  cpp::result<void, std::string> StoreFile(OpenAi::File& file_metadata,
                                           const char* content,
                                           uint64_t length) override;

  cpp::result<std::vector<OpenAi::File>, std::string> ListFiles(
      const std::string& purpose, uint8_t limit, const std::string& order,
      const std::string& after) const override;

  cpp::result<OpenAi::File, std::string> RetrieveFile(
      const std::string file_id) const override;

  cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
  RetrieveFileContent(const std::string& file_id) const override;

  cpp::result<std::pair<std::unique_ptr<char[]>, size_t>, std::string>
  RetrieveFileContentByPath(const std::string& path) const override;

  cpp::result<void, std::string> DeleteFileLocal(
      const std::string& file_id) override;

  explicit FileFsRepository(const std::filesystem::path& data_folder_path,
                            std::shared_ptr<DatabaseService> db_service)
      : data_folder_path_{data_folder_path}, db_service_(db_service) {
    CTL_INF("Constructing FileFsRepository..");
    auto file_container_path = data_folder_path_ / kFileContainerFolderName;

    if (!std::filesystem::exists(file_container_path)) {
      std::filesystem::create_directories(file_container_path);
    }
  }

  ~FileFsRepository() = default;

 private:
  std::filesystem::path GetFilePath() const;

  /**
   * The path to the data folder.
   */
  std::filesystem::path data_folder_path_;
  std::shared_ptr<DatabaseService> db_service_ = nullptr;
};
