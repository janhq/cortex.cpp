#pragma once

#include <filesystem>
#include <shared_mutex>
#include <unordered_map>
#include "common/repository/message_repository.h"

class MessageFsRepository : public MessageRepository {
  constexpr static auto kMessageFile = "messages.jsonl";
  constexpr static auto kThreadContainerFolderName = "threads";

 public:
  cpp::result<void, std::string> CreateMessage(
      OpenAi::Message& message) override;

  cpp::result<std::vector<OpenAi::Message>, std::string> ListMessages(
      const std::string& thread_id, uint8_t limit, const std::string& order,
      const std::string& after, const std::string& before,
      const std::string& run_id) const override;

  cpp::result<OpenAi::Message, std::string> RetrieveMessage(
      const std::string& thread_id,
      const std::string& message_id) const override;

  cpp::result<void, std::string> ModifyMessage(
      OpenAi::Message& message) override;

  cpp::result<void, std::string> DeleteMessage(
      const std::string& thread_id, const std::string& message_id) override;

  cpp::result<void, std::string> InitializeMessages(
      const std::string& thread_id,
      std::optional<std::vector<OpenAi::Message>> messages) override;

  explicit MessageFsRepository(const std::filesystem::path& data_folder_path)
      : data_folder_path_{data_folder_path} {
    CTL_INF("Constructing MessageFsRepository..");
    auto thread_container_path = data_folder_path_ / kThreadContainerFolderName;

    if (!std::filesystem::exists(thread_container_path)) {
      std::filesystem::create_directories(thread_container_path);
    }
  }

  ~MessageFsRepository() = default;

 private:
  cpp::result<std::vector<OpenAi::Message>, std::string> ReadMessageFromFile(
      const std::string& thread_id) const;

  /**
   * The path to the data folder.
   */
  std::filesystem::path data_folder_path_;

  std::filesystem::path GetMessagePath(const std::string& thread_id) const;

  std::shared_mutex* GrabMutex(const std::string& thread_id) const;

  mutable std::mutex mutex_map_mutex_;
  mutable std::unordered_map<std::string, std::unique_ptr<std::shared_mutex>>
      thread_mutexes_;
};
