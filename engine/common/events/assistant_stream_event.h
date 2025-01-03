#pragma once

#include <string>
#include "utils/result.hpp"

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
  AssistantStreamEvent(const std::string& event) : event{std::move(event)} {}

  AssistantStreamEvent(const AssistantStreamEvent&) = delete;

  AssistantStreamEvent& operator=(const AssistantStreamEvent&) = delete;

  AssistantStreamEvent(AssistantStreamEvent&& other) noexcept
      : event{std::move(other.event)} {}

  AssistantStreamEvent& operator=(AssistantStreamEvent&& other) noexcept {
    if (this != &other) {
      event = std::move(other.event);
    }
    return *this;
  }

  virtual ~AssistantStreamEvent() = default;

  std::string event;

  virtual auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> = 0;

  auto ToEvent() const -> cpp::result<std::string, std::string> {
    auto data = SingleLineJsonData();
    if (data.has_error()) {
      return cpp::fail(data.error());
    }
    return "event: " + event + "\n" + "data: " + data.value() + "\n\n";
  }
};
}  // namespace OpenAi
