#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunFailedEvent : public AssistantStreamEvent {
  std::string event{"thread.run.failed"};

  Run run;
};
}  // namespace OpenAi
