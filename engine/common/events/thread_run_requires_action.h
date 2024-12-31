#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run.h"

namespace OpenAi {
struct ThreadRunRequiresActionEvent : public AssistantStreamEvent {
  std::string event{"thread.run.requires_action"};

  Run run;
};
}  // namespace OpenAi
