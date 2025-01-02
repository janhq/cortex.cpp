#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunCancelledEvent : public AssistantStreamEvent {
  std::string event{"thread.run.cancelled"};

  Run run;
};
}  // namespace OpenAi
