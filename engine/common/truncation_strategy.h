#pragma once

#include <json/reader.h>
#include <json/value.h>
#include <string>
#include "common/json_serializable.h"
#include "utils/result.hpp"

namespace OpenAi {
struct TruncationStrategy : public JsonSerializable {
  TruncationStrategy() = default;

  TruncationStrategy(const TruncationStrategy&) = delete;

  TruncationStrategy& operator=(const TruncationStrategy&) = delete;

  TruncationStrategy(TruncationStrategy&& other) noexcept
      : type{std::move(other.type)},
        last_messages{std::move(other.last_messages)} {}

  TruncationStrategy& operator=(TruncationStrategy&& other) noexcept {
    if (this != &other) {
      type = std::move(other.type);
      last_messages = std::move(other.last_messages);
    }
    return *this;
  }

  /**
   * The truncation strategy to use for the thread. The default is auto.
   * If set to last_messages, the thread will be truncated to the n most
   * recent messages in the thread.
   *
   * When set to auto, messages in the middle of the thread will be dropped
   * to fit the context length of the model, max_prompt_tokens.
   */
  std::string type{"auto"};

  /**
   * The number of most recent messages from the thread when constructing
   * the context for the run.
   */
  std::optional<int> last_messages;

  static cpp::result<TruncationStrategy, std::string> FromJsonString(
      std::string&& json_str) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json_str, root)) {
      return cpp::fail("Failed to parse JSON: " +
                       reader.getFormattedErrorMessages());
    }

    try {
      return FromJson(std::move(root));
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJsonString failed: ") + e.what());
    }
  }

  static cpp::result<TruncationStrategy, std::string> FromJson(
      Json::Value&& json) {
    try {

      TruncationStrategy truncation_strategy;
      if (json.isMember("type") && json["type"].isString()) {
        truncation_strategy.type = json["type"].asString();
      }
      if (json.isMember("last_messages") && json["last_messages"].isInt()) {
        truncation_strategy.last_messages = json["last_messages"].asInt();
      }
      return truncation_strategy;

    } catch (const std::exception& e) {
      return cpp::fail("FromJson failed: " + std::string(e.what()));
    }
  }

  cpp::result<Json::Value, std::string> ToJson() const {
    Json::Value json;
    json["type"] = type;
    if (last_messages.has_value()) {
      json["last_messages"] = last_messages.value();
    }
    return json;
  }
};
}  // namespace OpenAi
