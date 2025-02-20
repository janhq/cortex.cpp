#pragma once

#include <stdexcept>
#include <string>
#include "common/assistant_code_interpreter_tool.h"
#include "common/assistant_file_search_tool.h"
#include "common/assistant_function_tool.h"
#include "common/assistant_tool.h"
#include "common/dto/base_dto.h"
#include "common/tool_resources.h"
#include "common/variant_map.h"
#include "utils/logging_utils.h"

namespace dto {
struct CreateAssistantDto : public BaseDto<CreateAssistantDto> {
  CreateAssistantDto() = default;

  ~CreateAssistantDto() = default;

  CreateAssistantDto(const CreateAssistantDto&) = delete;

  CreateAssistantDto& operator=(const CreateAssistantDto&) = delete;

  CreateAssistantDto(CreateAssistantDto&& other) noexcept
      : model{std::move(other.model)},
        name{std::move(other.name)},
        description{std::move(other.description)},
        instructions{std::move(other.instructions)},
        tools{std::move(other.tools)},
        tool_resources{std::move(other.tool_resources)},
        metadata{std::move(other.metadata)},
        temperature{std::move(other.temperature)},
        top_p{std::move(other.top_p)},
        response_format{std::move(other.response_format)} {}

  CreateAssistantDto& operator=(CreateAssistantDto&& other) noexcept {
    if (this != &other) {
      model = std::move(other.model);
      name = std::move(other.name);
      description = std::move(other.description);
      instructions = std::move(other.instructions);
      tools = std::move(other.tools);
      tool_resources = std::move(other.tool_resources),
      metadata = std::move(other.metadata);
      temperature = std::move(other.temperature);
      top_p = std::move(other.top_p);
      response_format = std::move(other.response_format);
    }
    return *this;
  }

  std::string model;

  std::optional<std::string> name;

  std::optional<std::string> description;

  std::optional<std::string> instructions;

  /**
   * A list of tool enabled on the assistant. There can be a maximum of
   * 128 tools per assistant. Tools can be of types code_interpreter,
   * file_search, or function.
   */
  std::vector<std::unique_ptr<OpenAi::AssistantTool>> tools;

  /**
   * A set of resources that are used by the assistant's tools. The resources
   * are specific to the type of tool. For example, the code_interpreter tool
   * requires a list of file IDs, while the file_search tool requires a list
   * of vector store IDs.
   */
  std::unique_ptr<OpenAi::ToolResources> tool_resources;

  std::optional<Cortex::VariantMap> metadata;

  std::optional<float> temperature;

  std::optional<float> top_p;

  std::optional<std::variant<std::string, Json::Value>> response_format;

  cpp::result<void, std::string> Validate() const override {
    if (model.empty()) {
      return cpp::fail("Model is mandatory");
    }

    if (response_format.has_value()) {
      const auto& variant_value = response_format.value();
      if (std::holds_alternative<std::string>(variant_value)) {
        if (std::get<std::string>(variant_value) != "auto") {
          return cpp::fail("Invalid response_format");
        }
      }
    }

    return {};
  }

  static CreateAssistantDto FromJson(Json::Value&& root) {
    if (root.empty()) {
      throw std::runtime_error("Json passed in FromJson can't be empty");
    }
    CreateAssistantDto dto;
    dto.model = std::move(root["model"].asString());
    if (root.isMember("name")) {
      dto.name = std::move(root["name"].asString());
    }
    if (root.isMember("description")) {
      dto.description = std::move(root["description"].asString());
    }
    if (root.isMember("instructions")) {
      dto.instructions = std::move(root["instructions"].asString());
    }
    if (root["metadata"].isObject() && !root["metadata"].empty()) {
      auto res = Cortex::ConvertJsonValueToMap(root["metadata"]);
      if (res.has_error()) {
        CTL_WRN("Failed to convert metadata to map: " + res.error());
      } else {
        dto.metadata = std::move(res.value());
      }
    }
    if (root.isMember("temperature")) {
      dto.temperature = root["temperature"].asFloat();
    }
    if (root.isMember("top_p")) {
      dto.top_p = root["top_p"].asFloat();
    }
    if (root.isMember("tools") && root["tools"].isArray()) {
      auto tools_array = root["tools"];
      for (const auto& tool : tools_array) {
        if (!tool.isMember("type") || !tool["type"].isString()) {
          CTL_WRN("Tool missing type field or invalid type");
          continue;
        }

        std::string tool_type = tool["type"].asString();
        if (tool_type == "file_search") {
          auto result = OpenAi::AssistantFileSearchTool::FromJson(tool);
          if (result.has_value()) {
            dto.tools.push_back(
                std::make_unique<OpenAi::AssistantFileSearchTool>(
                    std::move(result.value())));
          } else {
            CTL_WRN("Failed to parse file_search tool: " + result.error());
          }
        } else if (tool_type == "code_interpreter") {
          auto result = OpenAi::AssistantCodeInterpreterTool::FromJson();
          if (result.has_value()) {
            dto.tools.push_back(
                std::make_unique<OpenAi::AssistantCodeInterpreterTool>(
                    std::move(result.value())));
          } else {
            CTL_WRN("Failed to parse code_interpreter tool: " + result.error());
          }
        } else if (tool_type == "function") {
          auto result = OpenAi::AssistantFunctionTool::FromJson(tool);
          if (result.has_value()) {
            dto.tools.push_back(std::make_unique<OpenAi::AssistantFunctionTool>(
                std::move(result.value())));
          } else {
            CTL_WRN("Failed to parse function tool: " + result.error());
          }
        } else {
          CTL_WRN("Unknown tool type: " + tool_type);
        }
      }
    }
    if (root.isMember("tool_resources") && root["tool_resources"].isObject()) {
      const auto& tool_resources_json = root["tool_resources"];

      // Parse code interpreter resources
      if (tool_resources_json.isMember("code_interpreter")) {
        auto result = OpenAi::CodeInterpreter::FromJson(
            tool_resources_json["code_interpreter"]);
        if (result.has_value()) {
          dto.tool_resources = std::make_unique<OpenAi::CodeInterpreter>(
              std::move(result.value()));
        } else {
          CTL_WRN("Failed to parse code_interpreter resources: " +
                  result.error());
        }
      }

      // Parse file search resources
      if (tool_resources_json.isMember("file_search")) {
        auto result =
            OpenAi::FileSearch::FromJson(tool_resources_json["file_search"]);
        if (result.has_value()) {
          dto.tool_resources =
              std::make_unique<OpenAi::FileSearch>(std::move(result.value()));
        } else {
          CTL_WRN("Failed to parse file_search resources: " + result.error());
        }
      }
    }
    if (root.isMember("response_format")) {
      const auto& response_format = root["response_format"];
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
  }
};
}  // namespace dto
