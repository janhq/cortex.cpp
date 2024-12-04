#include <optional>
#include "common/message_attachment.h"
#include "utils/result.hpp"

namespace ThreadMessage {
inline cpp::result<Attachment, std::string> ParseAttachment(
    Json::Value&& json) {
  if (json.empty()) {
    return cpp::fail("Json string is empty");
  }

  Attachment attachment;
  attachment.file_id = json["file_id"].asString();

  std::vector<Tool> tools{};
  if (json["tools"].isArray()) {
    for (auto& tool_json : json["tools"]) {
      Tool tool{tool_json["type"].asString()};
      tools.push_back(tool);
    }
  }
  attachment.tools = tools;

  return attachment;
}

inline cpp::result<std::optional<std::vector<Attachment>>, std::string>
ParseAttachments(Json::Value&& json) {
  if (json.empty()) {
    // still count as success
    return std::nullopt;
  }
  if (!json.isArray()) {
    return cpp::fail("Json is not an array");
  }

  std::vector<Attachment> attachments;
  for (auto& attachment_json : json) {
    auto attachment = ParseAttachment(std::move(attachment_json));
    if (attachment.has_error()) {
      return cpp::fail(attachment.error());
    }
    attachments.push_back(attachment.value());
  }

  return attachments;
}
};  // namespace ThreadMessage
