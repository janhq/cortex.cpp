#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ThreadMessageInProgressEvent : public AssistantStreamEvent {
  ThreadMessageInProgressEvent(const std::string& json_message)
      : AssistantStreamEvent("thread.message.in_progress"),
        json_message{std::move(json_message)} {}

  ~ThreadMessageInProgressEvent() = default;

  std::string json_message;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return json_message;
  }
};
}  // namespace OpenAi
