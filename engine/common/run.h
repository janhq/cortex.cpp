#pragma once

#include <json/writer.h>
#include <string>
#include "common/assistant_code_interpreter_tool.h"
#include "common/assistant_file_search_tool.h"
#include "common/assistant_function_tool.h"
#include "common/assistant_tool.h"
#include "common/json_serializable.h"
#include "common/last_error.h"
#include "common/message_incomplete_detail.h"
#include "common/required_action.h"
#include "common/run_usage.h"
#include "common/tool_choice.h"
#include "common/truncation_strategy.h"
#include "common/variant_map.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

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

inline std::string RunStatusToString(RunStatus status) {
  switch (status) {
    case RunStatus::QUEUED:
      return "queued";
    case RunStatus::IN_PROGRESS:
      return "in_progress";
    case RunStatus::REQUIRES_ACTION:
      return "requires_action";
    case RunStatus::CANCELLING:
      return "cancelling";
    case RunStatus::CANCELLED:
      return "cancelled";
    case RunStatus::FAILED:
      return "failed";
    case RunStatus::COMPLETED:
      return "completed";
    case RunStatus::INCOMPLETE:
      return "incomplete";
    case RunStatus::EXPIRED:
      return "expired";
    default:
      return "completed";
  }
}

inline RunStatus RunStatusFromString(const std::string& input) {
  if (string_utils::EqualsIgnoreCase(input, "queued")) {
    return RunStatus::QUEUED;
  } else if (string_utils::EqualsIgnoreCase(input, "in_progress")) {
    return RunStatus::IN_PROGRESS;
  } else if (string_utils::EqualsIgnoreCase(input, "requires_action")) {
    return RunStatus::REQUIRES_ACTION;
  } else if (string_utils::EqualsIgnoreCase(input, "cancelling")) {
    return RunStatus::CANCELLING;
  } else if (string_utils::EqualsIgnoreCase(input, "cancelled")) {
    return RunStatus::CANCELLED;
  } else if (string_utils::EqualsIgnoreCase(input, "failed")) {
    return RunStatus::FAILED;
  } else if (string_utils::EqualsIgnoreCase(input, "incomplete")) {
    return RunStatus::INCOMPLETE;
  } else if (string_utils::EqualsIgnoreCase(input, "expired")) {
    return RunStatus::EXPIRED;
  } else {
    return RunStatus::COMPLETED;
  }
}

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
  std::optional<LastError> last_error;

  /**
   * The Unix timestamp (in seconds) for when the run will expire.
   */
  std::optional<uint32_t> expired_at;

  /**
   * The Unix timestamp (in seconds) for when the run was started.
   */
  std::optional<uint32_t> started_at;

  /**
   * The Unix timestamp (in seconds) for when the run was cancelled.
   */
  std::optional<uint32_t> cancelled_at;

  /**
   * The Unix timestamp (in seconds) for when the run failed.
   */
  std::optional<uint32_t> failed_at;

  /**
   * The Unix timestamp (in seconds) for when the run was completed.
   */
  std::optional<uint32_t> completed_at;

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

  std::variant<std::string, OpenAi::ToolChoice> tool_choice;

  bool parallel_tool_calls;

  std::variant<std::string, Json::Value> response_format;

  cpp::result<Json::Value, std::string> ToJson() const override {
    try {
      Json::Value root;

      root["id"] = id;
      root["object"] = object;
      root["created_at"] = created_at;
      root["thread_id"] = thread_id;
      root["assistant_id"] = assistant_id;
      root["status"] = RunStatusToString(status);

      if (required_action) {
        auto result = required_action->ToJson();
        if (result.has_value()) {
          root["required_action"] = result.value();
        }
      }

      if (last_error) {
        auto result = last_error->ToJson();
        if (result.has_value()) {
          root["last_error"] = result.value();
        }
      }

      if (expired_at)
        root["expired_at"] = *expired_at;
      if (started_at)
        root["started_at"] = *started_at;
      if (cancelled_at)
        root["cancelled_at"] = *cancelled_at;
      if (failed_at)
        root["failed_at"] = *failed_at;
      if (completed_at)
        root["completed_at"] = *completed_at;

      if (incomplete_detail) {
        auto result = incomplete_detail->ToJson();
        if (result.has_value()) {
          root["incomplete_detail"] = result.value();
        }
      }

      root["model"] = model;
      if (!instructions.empty()) {
        root["instructions"] = instructions;
      }

      // Handle tools array
      Json::Value tools_array(Json::arrayValue);
      for (const auto& tool : tools) {
        if (auto result = tool->ToJson(); result.has_value()) {
          tools_array.append(result.value());
        }
      }
      root["tools"] = tools_array;

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

      if (usage) {
        auto result = usage->ToJson();
        if (result.has_value()) {
          root["usage"] = result.value();
        }
      }

      if (temperature)
        root["temperature"] = *temperature;
      if (top_p)
        root["top_p"] = *top_p;
      if (max_prompt_tokens)
        root["max_prompt_tokens"] = *max_prompt_tokens;
      if (max_completion_tokens)
        root["max_completion_tokens"] = *max_completion_tokens;

      auto truncation_result = truncation_strategy.ToJson();
      if (truncation_result.has_value()) {
        root["truncation_strategy"] = truncation_result.value();
      } else {
        return cpp::fail("Failed to convert truncation_strategy to JSON: " +
                         truncation_result.error());
      }
      root["parallel_tool_calls"] = parallel_tool_calls;

      // Handle tool_choice variant
      if (std::holds_alternative<std::string>(tool_choice)) {
        root["tool_choice"] = std::get<std::string>(tool_choice);
      } else {
        auto& tc = std::get<OpenAi::ToolChoice>(tool_choice);
        auto result = tc.ToJson();
        if (result.has_value()) {
          root["tool_choice"] = result.value();
        }
      }

      // Handle response_format variant
      if (std::holds_alternative<std::string>(response_format)) {
        root["response_format"] = std::get<std::string>(response_format);
      } else {
        root["response_format"] = std::get<Json::Value>(response_format);
      }

      return root;
    } catch (const std::exception& e) {
      return cpp::fail("ToJson failed: " + std::string(e.what()));
    }
  }

  static std::vector<std::unique_ptr<AssistantTool>> ToolsFromJsonString(
      std::string&& json_str) {
    Json::Value json(Json::arrayValue);
    Json::Reader reader;
    std::vector<std::unique_ptr<AssistantTool>> tools;
    if (!reader.parse(json_str, json)) {
      return tools;
    }

    for (const auto& tool : json) {
      if (!tool.isMember("type") || !tool["type"].isString()) {
        CTL_WRN("Tool missing type field or invalid type");
        continue;
      }

      std::string tool_type = tool["type"].asString();
      if (tool_type == "file_search") {
        auto result = AssistantFileSearchTool::FromJson(tool);
        if (result.has_value()) {
          tools.push_back(std::make_unique<AssistantFileSearchTool>(
              std::move(result.value())));
        } else {
          CTL_WRN("Failed to parse file_search tool: " + result.error());
        }
      } else if (tool_type == "code_interpreter") {
        auto result = AssistantCodeInterpreterTool::FromJson();
        if (result.has_value()) {
          tools.push_back(std::make_unique<AssistantCodeInterpreterTool>(
              std::move(result.value())));
        } else {
          CTL_WRN("Failed to parse code_interpreter tool: " + result.error());
        }
      } else if (tool_type == "function") {
        auto result = AssistantFunctionTool::FromJson(tool);
        if (result.has_value()) {
          tools.push_back(std::make_unique<AssistantFunctionTool>(
              std::move(result.value())));
        } else {
          CTL_WRN("Failed to parse function tool: " + result.error());
        }
      } else {
        CTL_WRN("Unknown tool type: " + tool_type);
      }
    }

    return tools;
  }

  static cpp::result<std::string, std::string> ToolsToJsonString(
      const std::vector<std::unique_ptr<AssistantTool>>& tools) {
    Json::Value array(Json::arrayValue);

    for (const auto& tool : tools) {
      auto json_result = tool->ToJson();
      if (json_result.has_error()) {
        return cpp::fail("Failed to convert tool to JSON: " +
                         json_result.error());
      }
      array.append(json_result.value());
    }

    return array.toStyledString();
  }

  cpp::result<std::string, std::string> ToSingleLineJsonString(
      bool add_new_line = true) const {
    auto json = ToJson();
    if (json.has_error()) {
      return cpp::fail(json.error());
    }

    Json::FastWriter writer;
    try {
      if (add_new_line) {
        return writer.write(json.value());
      } else {
        auto json_str = writer.write(json.value());
        if (!json_str.empty() && json_str.back() == '\n') {
          json_str.pop_back();
        }
        return json_str;
      }
    } catch (const std::exception& e) {
      return cpp::fail(std::string("Failed to write JSON: ") + e.what());
    }
  }
};
}  // namespace OpenAi
