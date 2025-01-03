#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunInProgressEvent : public AssistantStreamEvent {
  std::string event{"thread.run.in_progress"};

  Run run;
};
}  // namespace OpenAi
