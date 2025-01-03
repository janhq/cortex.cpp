#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunQueuedEvent : public AssistantStreamEvent {
  std::string event{"thread.run.queued"};

  Run run;
};
}  // namespace OpenAi
