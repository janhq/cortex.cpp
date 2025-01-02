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

struct RunStepDelta {
  struct Delta {
    /**
     * The details of the run step.
     */
    std::unique_ptr<RunStepDetails> step_details;
  };

  /**
   * The identifier of the run step, which can be referenced in API endpoints.
   */
  std::string id;

  /**
   * The object type, which is always thread.run.step.delta.
   */
  std::string object{"thread.run.step.delta"};

  /**
   * The delta containing the fields that have changed on the run step.
   */
  Delta delta;
};
}  // namespace OpenAi
