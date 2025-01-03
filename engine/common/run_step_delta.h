#pragma once

#include <string>
#include "common/run_step_detail.h"

namespace OpenAi {
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
