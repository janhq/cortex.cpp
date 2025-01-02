#pragma once

#include "common/events/assistant_stream_event.h"

namespace OpenAi {
struct ErrorEvent : public AssistantStreamEvent {
  std::string event{"done"};

  std::string data{"[DONE]"};
};
}  // namespace OpenAi
