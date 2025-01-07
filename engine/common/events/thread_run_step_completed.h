#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadRunStepCompletedEvent : public AssistantStreamEvent {
  ThreadRunStepCompletedEvent(const std::string& json_str)
      : AssistantStreamEvent{"thread.run.step.completed"},
        json_message{json_str} {}

  ~ThreadRunStepCompletedEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
