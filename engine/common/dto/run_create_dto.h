#pragma once

#include <string>
#include "common/additional_message.h"
#include "common/assistant_code_interpreter_tool.h"
#include "common/assistant_file_search_tool.h"
#include "common/assistant_function_tool.h"
#include "common/assistant_tool.h"
#include "common/dto/base_dto.h"
#include "common/tool_choice.h"
#include "common/truncation_strategy.h"
#include "common/variant_map.h"
#include "utils/logging_utils.h"

namespace dto {
struct RunCreateDto : public BaseDto<RunCreateDto> {
  RunCreateDto() = default;

  ~RunCreateDto() = default;

  RunCreateDto(const RunCreateDto&) = delete;

  RunCreateDto& operator=(const RunCreateDto&) = delete;

  RunCreateDto(RunCreateDto&& other) noexcept
      : assistant_id{std::move(other.assistant_id)},
        model{std::move(other.model)},
        instructions{std::move(other.instructions)},
        additional_instructions{std::move(other.additional_instructions)},
        additional_messages{std::move(other.additional_messages)},
        tools{std::move(other.tools)},
        metadata{std::move(other.metadata)},
        temperature{std::move(other.temperature)},
        top_p{std::move(other.top_p)},
        stream{std::move(other.stream)},
        max_prompt_tokens{std::move(other.max_prompt_tokens)},
        max_completion_tokens{std::move(other.max_completion_tokens)},
        truncation_strategy{std::move(other.truncation_strategy)},
        tool_choice{std::move(other.tool_choice)},
        parallel_tool_calls{std::move(other.parallel_tool_calls)},
        response_format{std::move(other.response_format)} {}

  RunCreateDto& operator=(RunCreateDto&& other) noexcept {
    if (this != &other) {
      assistant_id = std::move(other.assistant_id);
      model = std::move(other.model);
      instructions = std::move(other.instructions);
      additional_instructions = std::move(other.additional_instructions);
      additional_messages = std::move(other.additional_messages);
      tools = std::move(other.tools);
      metadata = std::move(other.metadata);
      temperature = std::move(other.temperature);
      top_p = std::move(other.top_p);
      stream = std::move(other.stream);
      max_prompt_tokens = std::move(other.max_prompt_tokens);
      max_completion_tokens = std::move(other.max_completion_tokens);
      truncation_strategy = std::move(other.truncation_strategy);
      tool_choice = std::move(other.tool_choice);
      parallel_tool_calls = std::move(other.parallel_tool_calls);
      response_format = std::move(other.response_format);
    }
    return *this;
  }

  /**
   * The ID of the assistant to use to execute this run.
   */
  std::string assistant_id;

  /**
   * The ID of the Model to be used to execute this run.
   * If a value is provided here, it will override the model associated with
   * the assistant. If not, the model associated with the assistant will be used.
   */
  std::optional<std::string> model;

  /**
   * Overrides the instructions of the assistant. This is useful for modifying
   * the behavior on a per-run basis.
   */
  std::optional<std::string> instructions;

  /**
   * Appends additional instructions at the end of the instructions for the run.
   * This is useful for modifying the behavior on a per-run basis without overriding
   * other instructions.
   */
  std::optional<std::string> additional_instructions;

  /**
   * Adds additional messages to the thread before creating the run.
   */
  std::optional<std::vector<OpenAi::AdditionalMessage>> additional_messages;

  /**
   * A list of tool enabled on the assistant. There can be a maximum of 128
   * tools per assistant. Tools can be of types code_interpreter, file_search,
   * or function.
   */
  std::optional<std::vector<std::unique_ptr<OpenAi::AssistantTool>>> tools;

  /**
   * Set of 16 key-value pairs that can be attached to an object.
   * This can be useful for storing additional information about the object
   * in a structured format. Keys can be a maximum of 64 characters long
   * and values can be a maximum of 512 characters long.
   */
  std::optional<Cortex::VariantMap> metadata;

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
   * If true, returns a stream of events that happen during the Run as
   * server-sent events, terminating when the Run enters a terminal
   * state with a data: [DONE] message.
   */
  std::optional<bool> stream;

  /**
   * The maximum number of prompt tokens that may be used over the
   * course of the run. The run will make a best effort to use only
   * the number of prompt tokens specified, across multiple turns of
   * the run. If the run exceeds the number of prompt tokens specified,
   * the run will end with status incomplete.
   *
   * See incomplete_details for more info.
   */
  std::optional<uint32_t> max_prompt_tokens;

  /**
   * The maximum number of completion tokens that may be used over the
   * course of the run. The run will make a best effort to use only the
   * number of completion tokens specified, across multiple turns of the
   * run. If the run exceeds the number of completion tokens specified,
   * the run will end with status incomplete.
   *
   * See incomplete_details for more info.
   */
  std::optional<uint32_t> max_completion_tokens;

  /**
   * Controls for how a thread will be truncated prior to the run.
   * Use this to control the intial context window of the run.
   */
  std::optional<OpenAi::TruncationStrategy> truncation_strategy;

  /**
   * Controls which (if any) tool is called by the model.
   * none means the model will not call any tools and instead generates a message.
   * auto is the default value and means the model can pick between generating a message 
   * or calling one or more tools.
   * required means the model must call one or more tools before responding to the user.
   * Specifying a particular tool like {"type": "file_search"} or {"type": "function", "function": {"name": "my_function"}} forces the model to call that tool.
   */
  std::optional<std::variant<std::string, OpenAi::ToolChoice>> tool_choice;

  /**
   * Whether to enable parallel function calling during tool use.
   */
  std::optional<bool> parallel_tool_calls{true};

  /**
   *
   */
  std::optional<std::variant<std::string, Json::Value>> response_format;

  cpp::result<void, std::string> Validate() const {
    if (assistant_id.empty()) {
      return cpp::fail("assistant_id is mandatory");
    }

    return {};
  }

  static cpp::result<RunCreateDto, std::string> FromJson(Json::Value&& json) {
    try {
      RunCreateDto dto;

      if (!json.isMember("assistant_id") || !json["assistant_id"].isString()) {
        return cpp::fail("Missing or invalid 'assistant_id' field");
      }
      dto.assistant_id = json["assistant_id"].asString();
      dto.model = json["model"].asString();
      dto.instructions = json["instructions"].asString();
      dto.additional_instructions = json["additional_instructions"].asString();

      if (json.isMember("additional_messages") &&
          json["additional_messages"].isArray()) {

        std::vector<OpenAi::AdditionalMessage> msgs;
        auto additional_messages_array = json["additional_messages"];
        for (auto& additional_message : additional_messages_array) {
          auto result = OpenAi::AdditionalMessage::FromJson(
              std::move(additional_message));
          if (result.has_value()) {
            msgs.push_back(std::move(result.value()));
          } else {
            CTL_WRN("Failed to parse additional message: " + result.error());
          }
        }
        if (!msgs.empty()) {
          dto.additional_messages = std::move(msgs);
        }
      }

      if (json.isMember("tools") && json["tools"].isArray()) {
        auto tools_array = json["tools"];
        std::vector<std::unique_ptr<OpenAi::AssistantTool>> parsed_tools;
        for (const auto& tool : tools_array) {
          if (!tool.isMember("type") || !tool["type"].isString()) {
            CTL_WRN("Tool missing type field or invalid type");
            continue;
          }

          auto tool_type = tool["type"].asString();

          if (tool_type == "file_search") {
            auto result = OpenAi::AssistantFileSearchTool::FromJson(tool);
            if (result.has_value()) {
              parsed_tools.push_back(
                  std::make_unique<OpenAi::AssistantFileSearchTool>(
                      std::move(result.value())));
            } else {
              CTL_WRN("Failed to parse file_search tool: " + result.error());
            }
          } else if (tool_type == "code_interpreter") {
            auto result = OpenAi::AssistantCodeInterpreterTool::FromJson();
            if (result.has_value()) {
              parsed_tools.push_back(
                  std::make_unique<OpenAi::AssistantCodeInterpreterTool>(
                      std::move(result.value())));
            } else {
              CTL_WRN("Failed to parse code_interpreter tool: " +
                      result.error());
            }
          } else if (tool_type == "function") {
            auto result = OpenAi::AssistantFunctionTool::FromJson(tool);
            if (result.has_value()) {
              parsed_tools.push_back(
                  std::make_unique<OpenAi::AssistantFunctionTool>(
                      std::move(result.value())));
            } else {
              CTL_WRN("Failed to parse function tool: " + result.error());
            }
          } else {
            CTL_WRN("Unknown tool type: " + tool_type);
          }
        }
        if (!parsed_tools.empty()) {
          dto.tools = std::move(parsed_tools);
        }
      }

      // Parse metadata
      if (json.isMember("metadata") && json["metadata"].isObject()) {
        auto res = Cortex::ConvertJsonValueToMap(json["metadata"]);
        if (res.has_value()) {
          dto.metadata = res.value();
        } else {
          CTL_WRN("Failed to convert metadata to map: " + res.error());
        }
      }

      if (json.isMember("temperature") && json["temperature"].isDouble()) {
        dto.temperature = json["temperature"].asFloat();
      }

      if (json.isMember("top_p") && json["top_p"].isDouble()) {
        dto.top_p = json["top_p"].asFloat();
      }

      if (json.isMember("stream") && json["stream"].isBool()) {
        dto.stream = json["stream"].asBool();
      }

      if (json.isMember("max_prompt_tokens") &&
          json["max_prompt_tokens"].isUInt()) {
        dto.max_prompt_tokens = json["max_prompt_tokens"].asUInt();
      }

      if (json.isMember("max_completion_tokens") &&
          json["max_completion_tokens"].isUInt()) {
        dto.max_completion_tokens = json["max_completion_tokens"].asUInt();
      }

      if (json.isMember("truncation_strategy") &&
          json["truncation_strategy"].isObject()) {
        dto.truncation_strategy =
            std::move(OpenAi::TruncationStrategy::FromJson(
                          std::move(json["truncation_strategy"]))
                          .value());
      }

      if (json.isMember("tool_choice")) {
        if (json["tool_choice"].isString()) {
          if (json["tool_choice"].asString() != "none" &&
              json["tool_choice"].asString() != "auto" &&
              json["tool_choice"].asString() != "required") {
            return cpp::fail(
                "tool_choice must be either none, auto or required");
          }

          dto.tool_choice = json["tool_choice"].asString();
        } else if (json["tool_choice"].isObject()) {
          dto.tool_choice = std::move(
              OpenAi::ToolChoice::FromJson(std::move(json["tool_choice"]))
                  .value());
        } else {
          return cpp::fail("tool_choice must be either a string or an object");
        }
      }

      if (json.isMember("parallel_tool_calls") &&
          json["parallel_tool_calls"].isBool()) {
        dto.parallel_tool_calls = json["parallel_tool_calls"].asBool();
      }

      if (json.isMember("response_format")) {
        const auto& response_format = json["response_format"];
        if (response_format.isString()) {
          dto.response_format = response_format.asString();
        } else if (response_format.isObject()) {
          dto.response_format = response_format;
        } else {
          throw std::runtime_error(
              "response_format must be either a string or an object");
        }
      }
      return dto;
    } catch (const std::exception& e) {
      return cpp::fail("FromJson failed: " + std::string(e.what()));
    }
  }
};
}  // namespace dto
