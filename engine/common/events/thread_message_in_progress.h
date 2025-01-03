#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/message.h"

namespace OpenAi {
struct ThreadMessageInProgressEvent : public AssistantStreamEvent {
  std::string event{"thread.message.in_progress"};

  Message message;
};
}  // namespace OpenAi
