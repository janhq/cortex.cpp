#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadMessageCreatedEvent : public AssistantStreamEvent {
  ThreadMessageCreatedEvent(const std::string& json_message)
      : AssistantStreamEvent("thread.message.created"),
        json_message{std::move(json_message)} {}

  ~ThreadMessageCreatedEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
