#pragma once

#include <string>
#include "common/message_content.h"
#include "common/message_role.h"
#include "utils/logging_utils.h"

namespace OpenAi {
struct MessageDelta : public JsonSerializable {
  struct Delta : public JsonSerializable {
    Delta(Role role, std::vector<std::unique_ptr<Content>> content)
        : role{role}, content{std::move(content)} {};

    Delta(const Delta&) = delete;

    Delta& operator=(const Delta&) = delete;

    Delta(Delta&& other) noexcept
        : role{other.role}, content{std::move(other.content)} {}

    Delta& operator=(Delta&& other) noexcept {
      if (this != &other) {
        role = other.role;
        content = std::move(other.content);
      }
      return *this;
    }

    ~Delta() = default;

    Role role;

    std::vector<std::unique_ptr<Content>> content;

    cpp::result<Json::Value, std::string> ToJson() const override {
      Json::Value json;
      json["role"] = RoleToString(role);
      Json::Value content_json_arr{Json::arrayValue};
      for (auto& child_content : content) {
        if (auto it = child_content->ToJson(); it.has_value()) {
          content_json_arr.append(it.value());
        } else {
          CTL_WRN("Failed to convert content to json: " + it.error());
        }
      }
      json["content"] = content_json_arr;
      return json;
    }
  };

  explicit MessageDelta(Delta&& delta) : delta{std::move(delta)} {}

  MessageDelta(const MessageDelta&) = delete;

  MessageDelta& operator=(const MessageDelta&) = delete;

  MessageDelta(MessageDelta&& other) noexcept
      : id{std::move(other.id)},
        object{std::move(other.object)},
        delta{std::move(other.delta)} {}

  MessageDelta& operator=(MessageDelta&& other) noexcept {
    if (this != &other) {
      id = std::move(other.id);
      object = std::move(other.object);
      delta = std::move(other.delta);
    }
    return *this;
  }

  ~MessageDelta() = default;

  /**
   * The identifier of the message, which can be referenced in API endpoints.
   */
  std::string id;

  /**
   * The object type, which is always thread.message.delta.
   */
  std::string object{"thread.message.delta"};

  /**
   * The delta containing the fields that have changed on the Message.
   */
  Delta delta;

  auto ToJson() const -> cpp::result<Json::Value, std::string> override {
    Json::Value json;
    json["id"] = id;
    json["object"] = object;
    json["delta"] = delta.ToJson().value();
    return json;
  }
};
}  // namespace OpenAi
