#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunCreatedEvent : public AssistantStreamEvent {
  std::string event{"thread.run.created"};

  Run run;
};
}  // namespace OpenAi
