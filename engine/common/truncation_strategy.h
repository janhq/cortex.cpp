#pragma once

#include <json/value.h>
#include <string>
#include "utils/result.hpp"

namespace OpenAi {
struct TruncationStrategy {
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
};
}  // namespace OpenAi
