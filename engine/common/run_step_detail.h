#pragma once

#include <string>

namespace OpenAi {
struct RunStepDetails {
  virtual ~RunStepDetails() = default;

  std::string type;
};

struct MessageCreationDetail : public RunStepDetails {
  struct MessageCreation {
    std::string message_id;
  };

  std::string type{"message_creation"};

  MessageCreation message_creation;
};

struct ToolCalls : public RunStepDetails {
  std::string type{"tool_calls"};
  // TODO: namh implement toolcalls later
};
}  // namespace OpenAi
