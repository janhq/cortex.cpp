#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/thread.h"

namespace OpenAi {
struct ThreadRunCreatedEvent : public AssistantStreamEvent {
  std::string event{"thread.created"};

  Thread thread;
};
}  // namespace OpenAi
