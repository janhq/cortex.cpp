#pragma once

#include <json/writer.h>
#include "common/events/assistant_stream_event.h"
#include "common/message_delta.h"

namespace OpenAi {
struct ThreadMessageDeltaEvent : public AssistantStreamEvent {
  explicit ThreadMessageDeltaEvent(MessageDelta::Delta&& delta_msg)
      : AssistantStreamEvent("thread.message.delta"),
        delta{std::move(delta_msg)} {}

  ThreadMessageDeltaEvent(const ThreadMessageDeltaEvent&) = delete;

  ThreadMessageDeltaEvent& operator=(const ThreadMessageDeltaEvent&) = delete;

  ThreadMessageDeltaEvent(ThreadMessageDeltaEvent&& other) noexcept
      : AssistantStreamEvent(std::move(other)), delta{std::move(other.delta)} {}

  ThreadMessageDeltaEvent& operator=(ThreadMessageDeltaEvent&& other) noexcept {
    if (this != &other) {
      AssistantStreamEvent::operator=(std::move(other));
      delta = std::move(other.delta);
    }
    return *this;
  }

  ~ThreadMessageDeltaEvent() = default;

  MessageDelta delta;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    Json::Value json;
    Json::FastWriter writer;
    try {
      std::string json_str = writer.write(json);
      if (!json_str.empty() && json_str.back() == '\n') {
        json_str.pop_back();
      }
      return json_str;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("Failed to write JSON: ") + e.what());
    }
  }
};
}  // namespace OpenAi
