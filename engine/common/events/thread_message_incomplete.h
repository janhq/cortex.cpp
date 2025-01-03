#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/message.h"

namespace OpenAi {
struct ThreadMessageIncompleteEvent : public AssistantStreamEvent {
  std::string event{"thread.message.incomplete"};

  Message message;
};
}  // namespace OpenAi
