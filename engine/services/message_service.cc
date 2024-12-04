#include "services/message_service.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "utils/ulid/ulid.hh"

cpp::result<ThreadMessage::Message, std::string> MessageService::CreateMessage(
    const std::string& thread_id, const ThreadMessage::Role& role,
    std::variant<std::string,
                 std::vector<std::unique_ptr<ThreadMessage::Content>>>&&
        content,
    std::optional<std::vector<ThreadMessage::Attachment>> attachments,
    std::optional<Cortex::VariantMap> metadata) {
  LOG_TRACE << "CreateMessage for thread " << thread_id;
  auto now = std::chrono::system_clock::now();
  auto seconds_since_epoch =
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
          .count();
  std::vector<std::unique_ptr<ThreadMessage::Content>> content_list{};
  // if content is string
  if (std::holds_alternative<std::string>(content)) {
    auto text_content = std::make_unique<ThreadMessage::TextContent>();
    text_content->text.value = std::get<std::string>(content);
    content_list.push_back(std::move(text_content));
  } else {
    content_list = std::move(
        std::get<std::vector<std::unique_ptr<ThreadMessage::Content>>>(
            content));
  }

  ulid::ULID ulid = ulid::Create(seconds_since_epoch, []() { return 4; });
  std::string str = ulid::Marshal(ulid);
  LOG_TRACE << "Generated message ID: " << str;

  ThreadMessage::Message msg;
  msg.id = str;
  msg.object = "thread.message";
  msg.created_at = 0;
  msg.thread_id = thread_id;
  msg.status = ThreadMessage::Status::COMPLETED;
  msg.completed_at = seconds_since_epoch;
  msg.incomplete_at = std::nullopt;
  msg.incomplete_details = std::nullopt;
  msg.role = role;
  msg.content = std::move(content_list);
  msg.assistant_id = std::nullopt;
  msg.run_id = std::nullopt;
  msg.attachments = attachments;
  msg.metadata = metadata.value_or(Cortex::VariantMap{});
  auto res = message_repository_->CreateMessage(msg);
  if (res.has_error()) {
    return cpp::fail("Failed to create message: " + res.error());
  } else {
    return msg;
  }
}

cpp::result<std::vector<ThreadMessage::Message>, std::string>
MessageService::ListMessages(const std::string& thread_id, uint8_t limit,
                             const std::string& order, const std::string& after,
                             const std::string& before,
                             const std::string& run_id) const {
  CTL_INF("ListMessages for thread " + thread_id);
  return message_repository_->ListMessages(thread_id);
}

cpp::result<ThreadMessage::Message, std::string>
MessageService::RetrieveMessage(const std::string& thread_id,
                                const std::string& message_id) const {
  CTL_INF("RetrieveMessage for thread " + thread_id);
  return message_repository_->RetrieveMessage(thread_id, message_id);
}

cpp::result<ThreadMessage::Message, std::string> MessageService::ModifyMessage(
    const std::string& thread_id, const std::string& message_id,
    std::optional<Cortex::VariantMap> metadata) {
  LOG_TRACE << "ModifyMessage for thread " << thread_id << ", message "
            << message_id;
  auto msg = RetrieveMessage(thread_id, message_id);
  if (msg.has_error()) {
    return cpp::fail("Failed to retrieve message: " + msg.error());
  }

  msg->metadata = metadata.value();
  auto ptr = &msg.value();

  auto res = message_repository_->ModifyMessage(msg.value());
  if (res.has_error()) {
    CTL_ERR("Failed to modify message: " + res.error());
    return cpp::fail("Failed to modify message: " + res.error());
  } else {
    return RetrieveMessage(thread_id, message_id);
  }
}

cpp::result<std::string, std::string> MessageService::DeleteMessage(
    const std::string& thread_id, const std::string& message_id) {
  LOG_TRACE << "DeleteMessage for thread " + thread_id;
  auto res = message_repository_->DeleteMessage(thread_id, message_id);
  if (res.has_error()) {
    LOG_ERROR << "Failed to delete message: " + res.error();
    return cpp::fail("Failed to delete message: " + res.error());
  } else {
    return message_id;
  }
}
