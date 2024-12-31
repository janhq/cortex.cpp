#pragma once

#include "common/events/assistant_stream_event.h"
#include "common/run_step.h"

namespace OpenAi {
struct ThreadRunStepFailedEvent : public AssistantStreamEvent {
  std::string event{"thread.run.step.failed"};

  RunStep run_step;
};
}  // namespace OpenAi
