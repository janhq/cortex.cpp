#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunCompletedEvent : public AssistantStreamEvent {
  std::string event{"thread.run.completed"};

  Run run;
};
}  // namespace OpenAi