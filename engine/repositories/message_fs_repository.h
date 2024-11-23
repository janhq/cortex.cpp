#pragma once

#include <shared_mutex>
#include <unordered_map>
#include "common/repository/message_repository.h"

class MessageFsRepository : public MessageRepository {
 public:
  cpp::result<void, std::string> CreateMessage(
      ThreadMessage::Message& message) override;

  cpp::result<std::vector<ThreadMessage::Message>, std::string> ListMessages(
      const std::string& thread_id, uint8_t limit = 20,
      const std::string& order = "desc", const std::string& after = "",
      const std::string& before = "",
      const std::string& run_id = "") const override;

  cpp::result<ThreadMessage::Message, std::string> RetrieveMessage(
      const std::string& thread_id,
      const std::string& message_id) const override;

  cpp::result<void, std::string> ModifyMessage(
      ThreadMessage::Message& message) override;

  cpp::result<void, std::string> DeleteMessage(
      const std::string& thread_id, const std::string& message_id) override;

  ~MessageFsRepository() = default;

 private:
  cpp::result<std::vector<ThreadMessage::Message>, std::string>
  ReadMessageFromFile(const std::string& thread_id) const;

  std::shared_mutex* GrabMutex(const std::string& thread_id) const;

  mutable std::unordered_map<std::string, std::unique_ptr<std::shared_mutex>>
      thread_mutexes_;
  mutable std::mutex mutex_map_mutex_;
};
