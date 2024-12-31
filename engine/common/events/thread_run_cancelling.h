#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunCancellingEvent : public AssistantStreamEvent {
  std::string event{"thread.run.cancelling"};

  Run run;
};
}  // namespace OpenAi
