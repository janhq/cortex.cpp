#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunExpiredEvent : public AssistantStreamEvent {
  std::string event{"thread.run.expired"};

  Run run;
};
}  // namespace OpenAi
