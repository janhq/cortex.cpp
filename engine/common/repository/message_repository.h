#pragma once

#include "common/message.h"
#include "utils/result.hpp"

class MessageRepository {
 public:
  virtual cpp::result<void, std::string> CreateMessage(
      const OpenAi::Message& message) = 0;

  virtual cpp::result<std::vector<OpenAi::Message>, std::string> ListMessages(
      const std::string& thread_id, uint8_t limit, const std::string& order,
      const std::string& after, const std::string& before,
      const std::string& run_id) const = 0;

  virtual cpp::result<OpenAi::Message, std::string> RetrieveMessage(
      const std::string& thread_id, const std::string& message_id) const = 0;

  virtual cpp::result<void, std::string> ModifyMessage(
      const OpenAi::Message& message) = 0;

  virtual cpp::result<void, std::string> DeleteMessage(
      const std::string& thread_id, const std::string& message_id) = 0;

  virtual cpp::result<void, std::string> InitializeMessages(
      const std::string& thread_id,
      std::optional<std::vector<OpenAi::Message>> messages) = 0;

  virtual ~MessageRepository() = default;
};
