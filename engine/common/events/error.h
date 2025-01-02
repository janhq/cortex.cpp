#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ErrorEvent : public AssistantStreamEvent {
  std::string event{"error"};

  std::string error;
};
}  // namespace OpenAi
