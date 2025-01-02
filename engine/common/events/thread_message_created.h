#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/message.h"

namespace OpenAi {
struct ThreadMessageCreatedEvent : public AssistantStreamEvent {
  std::string event{"thread.message.created"};

  Message message;
};
}  // namespace OpenAi
