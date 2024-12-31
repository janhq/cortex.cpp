#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run_step.h"

namespace OpenAi {
struct ThreadRunStepInProgressEvent : public AssistantStreamEvent {
  std::string event{"thread.run.step.in_progress"};

  RunStep run_step;
};
}  // namespace OpenAi
