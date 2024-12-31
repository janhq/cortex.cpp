#pragma once

#include <optional>
#include <variant>
#include "common/message_attachment.h"
#include "common/message_attachment_factory.h"
#include "common/message_content.h"
#include "common/message_content_factory.h"
#include "common/message_role.h"
#include "common/variant_map.h"

namespace OpenAi {
struct AdditionalMessage {
  AdditionalMessage() = default;

  AdditionalMessage(const AdditionalMessage&) = delete;

  AdditionalMessage& operator=(const AdditionalMessage&) = delete;

  AdditionalMessage(AdditionalMessage&& other) noexcept
      : role{std::move(other.role)},
        content{std::move(other.content)},
        attachments{std::move(other.attachments)},
        metadata{std::move(other.metadata)} {}

  AdditionalMessage& operator=(AdditionalMessage&& other) noexcept {
    if (this != &other) {
      role = std::move(other.role);
      content = std::move(other.content);
      attachments = std::move(other.attachments);
      metadata = std::move(other.metadata);
    }

    return *this;
  }

  /**
   * The role of the entity that is creating the message.
   * Allowed values include: User or Assistant.
   */
  Role role;

  std::variant<std::string, std::vector<std::unique_ptr<OpenAi::Content>>>
      content;

  /**
   * A list of files attached to the message, and the tools they were added to.
   */
  std::optional<std::vector<Attachment>> attachments;

  /**
   * Set of 16 key-value pairs that can be attached to an object. This can be useful
   * for storing additional information about the object in a structured format.
   * Keys can be a maximum of 64 characters long and values can be a maximum of
   * 512 characters long.
   */
  std::optional<Cortex::VariantMap> metadata;

  static cpp::result<AdditionalMessage, std::string> FromJson(
      Json::Value&& json) {
    try {
      AdditionalMessage msg;
      if (json.isMember("role") && json["role"].isString()) {
        msg.role = RoleFromString(json["role"].asString());
      }
      if (!json.isMember("content")) {
        return cpp::fail("content is mandatory");
      }
      if (json["content"].isString()) {
        msg.content = std::move(json["content"].asString());
      } else if (json["content"].isArray()) {
        auto result = ParseContents(std::move(json["content"]));
        if (result.has_error()) {
          return cpp::fail("Failed to parse content array: " + result.error());
        }
        if (result.value().empty()) {
          return cpp::fail("Content array cannot be empty");
        }
        msg.content = std::move(result.value());
      } else {
        return cpp::fail("content must be either a string or an array");
      }

      if (json.isMember("attachments")) {
        msg.attachments =
            ParseAttachments(std::move(json["attachments"])).value();
      }
      if (json.isMember("metadata") && json["metadata"].isObject() &&
          !json["metadata"].empty()) {
        auto res = Cortex::ConvertJsonValueToMap(json["metadata"]);
        if (res.has_error()) {
          CTL_WRN("Failed to convert metadata to map: " + res.error());
        } else {
          msg.metadata = res.value();
        }
      }
      return msg;
    } catch (const std::exception& e) {
      return cpp::fail("FromJson failed: " + std::string(e.what()));
    }
  }
};
}  // namespace OpenAi
