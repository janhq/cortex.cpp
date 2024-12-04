#pragma once

#include "common/repository/message_repository.h"
#include "common/variant_map.h"
#include "utils/result.hpp"

class MessageService {
 public:
  explicit MessageService(std::shared_ptr<MessageRepository> message_repository)
      : message_repository_{message_repository} {}

  cpp::result<ThreadMessage::Message, std::string> CreateMessage(
      const std::string& thread_id, const ThreadMessage::Role& role,
      std::variant<std::string,
                   std::vector<std::unique_ptr<ThreadMessage::Content>>>&&
          content,
      std::optional<std::vector<ThreadMessage::Attachment>> attachments,
      std::optional<Cortex::VariantMap> metadata);

  cpp::result<std::vector<ThreadMessage::Message>, std::string> ListMessages(
      const std::string& thread_id, uint8_t limit = 20,
      const std::string& order = "desc", const std::string& after = "",
      const std::string& before = "", const std::string& run_id = "") const;

  cpp::result<ThreadMessage::Message, std::string> RetrieveMessage(
      const std::string& thread_id, const std::string& message_id) const;

  cpp::result<ThreadMessage::Message, std::string> ModifyMessage(
      const std::string& thread_id, const std::string& message_id,
      std::optional<std::unordered_map<
          std::string, std::variant<std::string, bool, uint64_t, double>>>
          metadata);

  cpp::result<std::string, std::string> DeleteMessage(
      const std::string& thread_id, const std::string& message_id);

 private:
  std::shared_ptr<MessageRepository> message_repository_;
};
