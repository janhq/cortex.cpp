#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadRunCompletedEvent : public AssistantStreamEvent {
  ThreadRunCompletedEvent(const std::string& json_str)
      : AssistantStreamEvent{"thread.run.completed"}, json_message{json_str} {}

  ~ThreadRunCompletedEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
