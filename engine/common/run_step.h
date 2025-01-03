#pragma once

#include <string>
#include "common/last_error.h"
#include "common/run_step_detail.h"
#include "common/run_usage.h"
#include "common/variant_map.h"

namespace OpenAi {

enum class RunStepStatus { IN_PROGRESS, CANCELLED, FAILED, COMPLETED, EXPIRED };

enum class RunStepType { MESSAGE_CREATION, TOOL_CALLS };

struct RunStep {
  std::string id;

  std::string object{"thread.run.step"};

  uint32_t created_at;

  std::string assistant_id;

  std::string thread_id;

  std::string run_id;

  RunStepType type;

  RunStepStatus status;

  /**
   * The details of the run step.
   */
  std::unique_ptr<RunStepDetails> step_details;

  /**
   * The last error associated with this run. Will be null if there are no errors.
   */
  std::optional<LastError> last_error{std::nullopt};

  /**
   * The Unix timestamp (in seconds) for when the run will expire.
   */
  uint32_t expired_at;

  /**
   * The Unix timestamp (in seconds) for when the run was cancelled.
   */
  uint32_t cancelled_at;

  /**
   * The Unix timestamp (in seconds) for when the run failed.
   */
  uint32_t failed_at;

  /**
   * The Unix timestamp (in seconds) for when the run was completed.
   */
  uint32_t completed_at;

  /**
   * Set of 16 key-value pairs that can be attached to an object. This can
   * be useful for storing additional information about the object in a
   * structured format. Keys can be a maximum of 64 characters long and
   * values can be a maximum of 512 characters long.
   */
  Cortex::VariantMap metadata;

  /**
   * Usage statistics related to the run. This value will be null if the run
   * is not in a terminal state (i.e. in_progress, queued, etc.).
   */
  std::optional<RunUsage> usage;
};
}  // namespace OpenAi
