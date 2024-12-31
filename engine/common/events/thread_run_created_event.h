#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadRunCreatedEvent : public AssistantStreamEvent {
  ThreadRunCreatedEvent(const std::string& json_str)
      : AssistantStreamEvent{"thread.run.created"}, json_message{json_str} {}

  ~ThreadRunCreatedEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
