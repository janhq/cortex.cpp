#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadRunInProgressEvent : public AssistantStreamEvent {
  ThreadRunInProgressEvent(const std::string& json_str)
      : AssistantStreamEvent{"thread.run.in_progress"},
        json_message{json_str} {}

  ~ThreadRunInProgressEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
