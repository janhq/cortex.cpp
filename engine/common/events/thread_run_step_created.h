#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadRunStepCreatedEvent : public AssistantStreamEvent {
  ThreadRunStepCreatedEvent(const std::string& json_str)
      : AssistantStreamEvent{"thread.run.step.created"},
        json_message{json_str} {}

  ~ThreadRunStepCreatedEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
