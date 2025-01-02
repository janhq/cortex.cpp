#pragma once

#include <string>

namespace OpenAi {
struct AssistantStreamEvent {
  virtual ~AssistantStreamEvent() = default;

  std::string event;
};
}  // namespace OpenAi
