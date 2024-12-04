#pragma once

#include "common/message.h"
#include "utils/result.hpp"

class MessageRepository {
 public:
  virtual cpp::result<void, std::string> CreateMessage(
      ThreadMessage::Message& message) = 0;

  virtual cpp::result<std::vector<ThreadMessage::Message>, std::string>
  ListMessages(const std::string& thread_id, uint8_t limit = 20,
               const std::string& order = "desc", const std::string& after = "",
               const std::string& before = "",
               const std::string& run_id = "") const = 0;

  virtual cpp::result<ThreadMessage::Message, std::string> RetrieveMessage(
      const std::string& thread_id, const std::string& message_id) const = 0;

  virtual cpp::result<void, std::string> ModifyMessage(
      ThreadMessage::Message& message) = 0;

  virtual cpp::result<void, std::string> DeleteMessage(
      const std::string& thread_id, const std::string& message_id) = 0;

  virtual ~MessageRepository() = default;
};
