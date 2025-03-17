#include "services/message_service.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "utils/ulid_generator.h"

cpp::result<OpenAi::Message, std::string> MessageService::CreateMessage(
    const std::string& thread_id, const OpenAi::Role& role,
    std::variant<std::string, std::vector<std::unique_ptr<OpenAi::Content>>>&&
        content,
    std::optional<std::vector<OpenAi::Attachment>> attachments,
    std::optional<Cortex::VariantMap> metadata) {
  LOG_TRACE << "CreateMessage for thread " << thread_id;

  uint32_t seconds_since_epoch =
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  std::vector<std::unique_ptr<OpenAi::Content>> content_list{};

  // if content is string
  if (std::holds_alternative<std::string>(content)) {
    auto text_content = std::make_unique<OpenAi::TextContent>();
    text_content->text.value = std::get<std::string>(content);
    content_list.push_back(std::move(text_content));
  } else {
    content_list = std::move(
        std::get<std::vector<std::unique_ptr<OpenAi::Content>>>(content));
  }

  OpenAi::Message msg;
  msg.id = ulid::GenerateUlid();
  msg.object = "thread.message";
  msg.created_at = seconds_since_epoch;
  msg.thread_id = thread_id;
  msg.status = OpenAi::Status::COMPLETED;
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

cpp::result<std::vector<OpenAi::Message>, std::string>
MessageService::ListMessages(const std::string& thread_id, uint8_t limit,
                             const std::string& order, const std::string& after,
                             const std::string& before,
                             const std::string& run_id) const {
  CTL_INF("ListMessages for thread " + thread_id);
  return message_repository_->ListMessages(thread_id, limit, order, after,
                                           before, run_id);
}

cpp::result<OpenAi::Message, std::string> MessageService::RetrieveMessage(
    const std::string& thread_id, const std::string& message_id) const {
  CTL_INF("RetrieveMessage for thread " + thread_id);
  return message_repository_->RetrieveMessage(thread_id, message_id);
}

cpp::result<OpenAi::Message, std::string> MessageService::ModifyMessage(
    const std::string& thread_id, const std::string& message_id,
    std::optional<Cortex::VariantMap> metadata,
    std::optional<std::variant<std::string,
                               std::vector<std::unique_ptr<OpenAi::Content>>>>
        content) {
  LOG_TRACE << "ModifyMessage for thread " << thread_id << ", message "
            << message_id;
  auto msg = RetrieveMessage(thread_id, message_id);
  if (msg.has_error()) {
    return cpp::fail("Failed to retrieve message: " + msg.error());
  }

  if (metadata.has_value()) {
    msg->metadata = metadata.value();
  }
  if (content.has_value()) {
    std::vector<std::unique_ptr<OpenAi::Content>> content_list{};

    // If content is string
    if (std::holds_alternative<std::string>(*content)) {
      auto text_content = std::make_unique<OpenAi::TextContent>();
      text_content->text.value = std::get<std::string>(*content);
      content_list.push_back(std::move(text_content));
    } else {
      content_list = std::move(
          std::get<std::vector<std::unique_ptr<OpenAi::Content>>>(*content));
    }

    msg->content = std::move(content_list);
  }
/*   auto ptr = &msg.value(); */

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

cpp::result<void, std::string> MessageService::InitializeMessages(
    const std::string& thread_id,
    std::optional<std::vector<OpenAi::Message>> messages) {
  CTL_INF("InitializeMessages for thread " + thread_id);

  if (messages.has_value()) {
    CTL_INF("Prepopulated messages length: " +
            std::to_string(messages->size()));
  } else {

    CTL_INF("Prepopulated with empty messages");
  }

  return message_repository_->InitializeMessages(thread_id,
                                                 std::move(messages));
}
