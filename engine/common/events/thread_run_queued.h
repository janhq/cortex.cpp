#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadRunQueuedEvent : public AssistantStreamEvent {
  ThreadRunQueuedEvent(const std::string& json_str)
      : AssistantStreamEvent{"thread.run.queued"}, json_message{json_str} {}

  ~ThreadRunQueuedEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
