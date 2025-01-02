#pragma once

#include <string>

namespace OpenAi {
/**
 * Represents an event emitted when streaming a Run.
 *
 * Each event in a server-sent events stream has an event and data property:
 * event: thread.created
 * data: {"id": "thread_123", "object": "thread", ...}
 *
 * We emit events whenever a new object is created, transitions to a new state,
 * or is being streamed in parts (deltas).
 *
 * For example, we emit thread.run.created when a new run is created,
 * thread.run.completed when a run completes, and so on. When an Assistant chooses
 * to create a message during a run, we emit a thread.message.created event,
 * a thread.message.in_progress event, many thread.message.delta events,
 * and finally a thread.message.completed event.
 */
struct AssistantStreamEvent {
  virtual ~AssistantStreamEvent() = default;

  std::string event;
};
}  // namespace OpenAi
