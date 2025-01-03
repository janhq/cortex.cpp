#pragma once

#include <json/value.h>
#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct DoneEvent : public AssistantStreamEvent {
  DoneEvent() : AssistantStreamEvent{"done"} {}

  ~DoneEvent() = default;

  std::string data{"[DONE]"};

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    return data;
  }
};
}  // namespace OpenAi
