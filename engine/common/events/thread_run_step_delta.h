#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run_step_delta.h"

namespace OpenAi {
struct ThreadRunStepDeltaEvent : public AssistantStreamEvent {
  std::string event{"thread.run.step.delta"};

  RunStepDelta delta;
};
}  // namespace OpenAi
