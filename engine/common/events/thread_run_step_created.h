#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run_step.h"

namespace OpenAi {
struct ThreadRunStepCreatedEvent : public AssistantStreamEvent {
  std::string event{"thread.run.step.created"};

  RunStep run_step;
};
}  // namespace OpenAi
