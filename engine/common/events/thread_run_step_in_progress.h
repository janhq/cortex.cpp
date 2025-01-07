#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadRunStepInProgressEvent : public AssistantStreamEvent {
  ThreadRunStepInProgressEvent(const std::string& json_str)
      : AssistantStreamEvent{"thread.run.step.in_progress"},
        json_message{json_str} {}

  ~ThreadRunStepInProgressEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
