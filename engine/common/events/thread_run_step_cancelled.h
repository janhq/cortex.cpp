#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run_step.h"

namespace OpenAi {
struct ThreadRunStepCancelledEvent : public AssistantStreamEvent {
  std::string event{"thread.run.step.cancelled"};

  RunStep run_step;
};
}  // namespace OpenAi
