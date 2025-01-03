#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/message.h"

namespace OpenAi {
struct ThreadMessageCompletedEvent : public AssistantStreamEvent {
  std::string event{"thread.message.completed"};

  Message message;
};
}  // namespace OpenAi
