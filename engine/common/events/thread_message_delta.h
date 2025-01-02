#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/message_delta.h"

namespace OpenAi {
struct ThreadMessageDeltaEvent : public AssistantStreamEvent {
  std::string event{"thread.message.delta"};

  MessageDelta delta;
};
}  // namespace OpenAi
