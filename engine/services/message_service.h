#pragma once

#include "common/repository/message_repository.h"
#include "common/variant_map.h"
#include "utils/result.hpp"

class MessageService {
 public:
  explicit MessageService(std::shared_ptr<MessageRepository> message_repository)
      : message_repository_{message_repository} {}

  cpp::result<OpenAi::Message, std::string> CreateMessage(
      const std::string& thread_id, const OpenAi::Role& role,
      std::variant<std::string, std::vector<std::unique_ptr<OpenAi::Content>>>&&
          content,
      std::optional<std::vector<OpenAi::Attachment>> attachments,
      std::optional<Cortex::VariantMap> metadata);

  cpp::result<void, std::string> InitializeMessages(
      const std::string& thread_id,
      std::optional<std::vector<OpenAi::Message>> messages);

  cpp::result<std::vector<OpenAi::Message>, std::string> ListMessages(
      const std::string& thread_id, uint8_t limit, const std::string& order,
      const std::string& after, const std::string& before,
      const std::string& run_id) const;

  cpp::result<OpenAi::Message, std::string> RetrieveMessage(
      const std::string& thread_id, const std::string& message_id) const;

  cpp::result<OpenAi::Message, std::string> ModifyMessage(
      const std::string& thread_id, const std::string& message_id,
      std::optional<Cortex::VariantMap> metadata,
      std::optional<std::variant<std::string,
                                 std::vector<std::unique_ptr<OpenAi::Content>>>>
          content);

  cpp::result<std::string, std::string> DeleteMessage(
      const std::string& thread_id, const std::string& message_id);

 private:
  std::shared_ptr<MessageRepository> message_repository_;
};
