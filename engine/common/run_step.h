#pragma once

#include <string>
#include "common/last_error.h"
#include "common/run_step_detail.h"
#include "common/run_usage.h"
#include "common/variant_map.h"
#include "utils/string_utils.h"

namespace OpenAi {

enum class RunStepStatus { IN_PROGRESS, CANCELLED, FAILED, COMPLETED, EXPIRED };

inline std::string RunStepStatusToString(RunStepStatus status) {
  switch (status) {
    case RunStepStatus::IN_PROGRESS:
      return "in_progress";
    case RunStepStatus::CANCELLED:
      return "cancelled";
    case RunStepStatus::FAILED:
      return "failed";
    case RunStepStatus::COMPLETED:
      return "completed";
    default:
      return "expired";
  }
}

inline RunStepStatus RunStepStatusFromString(const std::string& input) {
  if (string_utils::EqualsIgnoreCase(input, "in_progress")) {
    return RunStepStatus::IN_PROGRESS;
  } else if (string_utils::EqualsIgnoreCase(input, "cancelled")) {
    return RunStepStatus::CANCELLED;
  } else if (string_utils::EqualsIgnoreCase(input, "failed")) {
    return RunStepStatus::FAILED;
  } else if (string_utils::EqualsIgnoreCase(input, "completed")) {
    return RunStepStatus::COMPLETED;
  } else {
    return RunStepStatus::EXPIRED;
  }
}

enum class RunStepType { MESSAGE_CREATION, TOOL_CALLS };

inline std::string RunStepTypeToString(RunStepType type) {
  switch (type) {
    case RunStepType::MESSAGE_CREATION:
      return "message_creation";
    default:
      return "tool_calls";
  }
}

inline RunStepType RunStepTypeFromString(const std::string& input) {
  if (string_utils::EqualsIgnoreCase(input, "message_creation")) {
    return RunStepType::MESSAGE_CREATION;
  } else {
    return RunStepType::TOOL_CALLS;
  }
}

struct RunStep : public JsonSerializable {
  /**
   * The identifier of the run step, which can be referenced in API endpoints.
   */
  std::string id;

  /**
   * The object type, which is always thread.run.step.
   */
  std::string object{"thread.run.step"};

  /**
   * The Unix timestamp (in seconds) for when the run step was created.
   */
  uint32_t created_at;

  /**
   * The ID of the assistant associated with the run step.
   */
  std::string assistant_id;

  /**
   * The ID of the thread the run and run steps belong to.
   */
  std::string thread_id;

  /**
   * The ID of the run that this run step is a part of.
   */
  std::string run_id;

  /**
   * The type of run step, which can be either message_creation or tool_calls.
   */
  RunStepType type;

  /**
   * The status of the run step, which can be either in_progress, cancelled, failed,
   * completed, or expired.
   */
  RunStepStatus status;

  /**
   * The details of the run step.
   */
  std::unique_ptr<RunStepDetails> step_details;

  /**
   * The last error associated with this run. Will be null if there are no errors.
   */
  std::optional<LastError> last_error;

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

  cpp::result<Json::Value, std::string> ToJson() const {
    try {
      Json::Value root;

      root["id"] = id;
      root["object"] = object;
      root["created_at"] = created_at;
      root["assistant_id"] = assistant_id;
      root["thread_id"] = thread_id;
      root["run_id"] = run_id;
      root["type"] = RunStepTypeToString(type);
      root["status"] = RunStepStatusToString(status);

      // Handle step_details
      if (step_details) {
        auto result = step_details->ToJson();
        if (result.has_error()) {
          return cpp::fail("Failed to convert step_details to JSON: " +
                           result.error());
        }
        root["step_details"] = result.value();
      }

      // Handle last_error
      if (last_error) {
        auto result = last_error->ToJson();
        if (result.has_error()) {
          return cpp::fail("Failed to convert last_error to JSON: " +
                           result.error());
        }
        root["last_error"] = result.value();
      }

      root["expired_at"] = expired_at;
      root["cancelled_at"] = cancelled_at;
      root["failed_at"] = failed_at;
      root["completed_at"] = completed_at;

      // Handle metadata
      Json::Value metadata_json(Json::objectValue);
      for (const auto& [key, value] : metadata) {
        if (std::holds_alternative<bool>(value)) {
          metadata_json[key] = std::get<bool>(value);
        } else if (std::holds_alternative<uint64_t>(value)) {
          metadata_json[key] = std::get<uint64_t>(value);
        } else if (std::holds_alternative<double>(value)) {
          metadata_json[key] = std::get<double>(value);
        } else {
          metadata_json[key] = std::get<std::string>(value);
        }
      }
      root["metadata"] = metadata_json;

      // Handle usage
      if (usage) {
        auto result = usage->ToJson();
        if (result.has_value()) {
          root["usage"] = result.value();
        } else {
          return cpp::fail("Failed to convert usage to JSON: " +
                           result.error());
        }
      }

      return root;
    } catch (const std::exception& e) {
      return cpp::fail("ToJson failed: " + std::string(e.what()));
    }
  }
};
}  // namespace OpenAi
