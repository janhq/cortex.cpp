#pragma once

#include <string>
#include "common/assistant_tool.h"
#include "common/json_serializable.h"
#include "common/last_error.h"
#include "common/required_action.h"
#include "common/run_usage.h"
#include "common/truncation_strategy.h"
#include "common/variant_map.h"

namespace OpenAi {

enum class RunStatus {
  QUEUED,
  IN_PROGRESS,
  REQUIRES_ACTION,
  CANCELLING,
  CANCELLED,
  FAILED,
  COMPLETED,
  INCOMPLETE,
  EXPIRED
};

struct IncompleteDetail : public JsonSerializable {
  std::string reason;

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value root;
    root["reason"] = reason;
    return root;
  }
};

struct Run : public JsonSerializable {
  Run() = default;

  ~Run() = default;

  Run(const Run&) = delete;

  Run& operator=(const Run&) = delete;

  Run(Run&& other) noexcept
      : id{std::move(other.id)},
        object{std::move(other.object)},
        created_at{other.created_at},
        thread_id{std::move(other.thread_id)},
        assistant_id{std::move(other.assistant_id)},
        status{std::move(other.status)},
        required_action{std::move(other.required_action)},
        last_error{std::move(other.last_error)},
        expired_at{other.expired_at},
        started_at{other.started_at},
        cancelled_at{other.cancelled_at},
        failed_at{other.failed_at},
        completed_at{other.completed_at},
        incomplete_detail{std::move(other.incomplete_detail)},
        model{std::move(other.model)},
        instructions{std::move(other.instructions)},
        tools{std::move(other.tools)},
        metadata{std::move(other.metadata)},
        usage{std::move(other.usage)},
        temperature{other.temperature},
        top_p{other.top_p},
        max_prompt_tokens{other.max_prompt_tokens},
        max_completion_tokens{other.max_completion_tokens},
        truncation_strategy{std::move(other.truncation_strategy)},
        tool_choice{std::move(other.tool_choice)},
        parallel_tool_calls{other.parallel_tool_calls},
        response_format{std::move(other.response_format)} {}

  Run& operator=(Run&& other) noexcept {
    if (this != &other) {
      id = std::move(other.id);
      object = std::move(other.object);
      created_at = other.created_at;
      thread_id = std::move(other.thread_id);
      assistant_id = std::move(other.assistant_id);
      status = std::move(other.status);
      required_action = std::move(other.required_action);
      last_error = std::move(other.last_error);
      expired_at = other.expired_at;
      started_at = other.started_at;
      cancelled_at = other.cancelled_at;
      failed_at = other.failed_at;
      completed_at = other.completed_at;
      incomplete_detail = std::move(other.incomplete_detail);
      model = std::move(other.model);
      instructions = std::move(other.instructions);
      tools = std::move(other.tools);
      metadata = std::move(other.metadata);
      usage = std::move(other.usage);
      temperature = other.temperature;
      top_p = other.top_p;
      max_prompt_tokens = other.max_prompt_tokens;
      max_completion_tokens = other.max_completion_tokens;
      truncation_strategy = std::move(other.truncation_strategy);
      tool_choice = std::move(other.tool_choice);
      parallel_tool_calls = other.parallel_tool_calls;
      response_format = std::move(other.response_format);
    }
    return *this;
  }

  /**
   * The identifier, which can be referenced in API endpoints.
   */
  std::string id;

  /**
   * The object type, which is always thread.run.
   */
  std::string object{"thread.run"};

  uint32_t created_at;

  std::string thread_id;

  std::string assistant_id;

  RunStatus status;

  /**
   * Details on the action required to continue the run. Will be null if no
   * action is required.
   */
  std::optional<RequiredAction> required_action;

  /**
   * The last error associated with this run. Will be null if there are no errors.
   */
  std::optional<LastError> last_error{std::nullopt};

  /**
   * The Unix timestamp (in seconds) for when the run will expire.
   */
  uint32_t expired_at;

  /**
   * The Unix timestamp (in seconds) for when the run was started.
   */
  uint32_t started_at;

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
   * Details on why the run is incomplete. Will be null if the run is
   * not incomplete.
   */
  std::optional<IncompleteDetail> incomplete_detail;

  /**
   * The model that the assistant used for this run.
   */
  std::string model;

  std::string instructions;

  std::vector<std::unique_ptr<AssistantTool>> tools;

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

  /**
   * What sampling temperature to use, between 0 and 2. Higher values like
   * 0.8 will make the output more random, while lower values like 0.2 will
   * make it more focused and deterministic.
   */
  std::optional<float> temperature;

  /**
   * An alternative to sampling with temperature, called nucleus sampling,
   * where the model considers the results of the tokens with top_p
   * probability mass. So 0.1 means only the tokens comprising the top 10%
   * probability mass are considered.
   *
   * We generally recommend altering this or temperature but not both.
   */
  std::optional<float> top_p;

  /**
   * The maximum number of prompt tokens specified to have been used over
   * the course of the run.
   */
  std::optional<uint32_t> max_prompt_tokens;

  /**
   * The maximum number of completion tokens specified to have been used over
   * the course of the run.
   */
  std::optional<uint32_t> max_completion_tokens;

  TruncationStrategy truncation_strategy;

  std::variant<std::string, Json::Value> tool_choice;

  bool parallel_tool_calls;

  std::variant<std::string, Json::Value> response_format;

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value root;

    return root;
  }
};
}  // namespace OpenAi
