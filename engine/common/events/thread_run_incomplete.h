#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunIncompleteEvent : public AssistantStreamEvent {
  std::string event{"thread.run.incomplete"};

  Run run;
};
}  // namespace OpenAi
