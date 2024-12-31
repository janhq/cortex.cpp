#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadMessageCompletedEvent : public AssistantStreamEvent {
  explicit ThreadMessageCompletedEvent(const std::string& json_message)
      : AssistantStreamEvent("thread.message.completed"),
        json_message{std::move(json_message)} {}

  ~ThreadMessageCompletedEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
