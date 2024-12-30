#pragma once

#include <string>
#include "common/assistant_code_interpreter_tool.h"
#include "common/assistant_file_search_tool.h"
#include "common/assistant_function_tool.h"
#include "common/assistant_tool.h"
#include "common/tool_resources.h"
#include "common/variant_map.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"

namespace OpenAi {
// Deprecated. After jan's migration, we should remove this struct
struct JanAssistant : JsonSerializable {
  std::string id;

  std::string name;

  std::string object = "assistant";

  uint32_t created_at;

  Json::Value tools;

  Json::Value model;

  std::string instructions;

  ~JanAssistant() = default;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;

      json["id"] = id;
      json["name"] = name;
      json["object"] = object;
      json["created_at"] = created_at;

      json["tools"] = tools;
      json["model"] = model;
      json["instructions"] = instructions;

      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }

  static cpp::result<JanAssistant, std::string> FromJson(Json::Value&& json) {
    if (json.empty()) {
      return cpp::fail("Empty JSON");
    }

    JanAssistant assistant;
    if (json.isMember("assistant_id")) {
      assistant.id = json["assistant_id"].asString();
    } else {
      assistant.id = json["id"].asString();
    }

    if (json.isMember("assistant_name")) {
      assistant.name = json["assistant_name"].asString();
    } else {
      assistant.name = json["name"].asString();
    }
    assistant.object = "assistant";
    assistant.created_at = 0;  // Jan does not have this
    if (json.isMember("tools")) {
      assistant.tools = json["tools"];
    }
    if (json.isMember("model")) {
      assistant.model = json["model"];
    }
    assistant.instructions = json["instructions"].asString();

    return assistant;
  }
};

struct Assistant : JsonSerializable {
  Assistant() = default;

  ~Assistant() = default;

  Assistant(const Assistant&) = delete;

  Assistant& operator=(const Assistant&) = delete;

  Assistant(Assistant&& other) noexcept
      : id{std::move(other.id)},
        object{std::move(other.object)},
        created_at{other.created_at},
        name{std::move(other.name)},
        description{std::move(other.description)},
        model(std::move(other.model)),
        instructions(std::move(other.instructions)),
        tools(std::move(other.tools)),
        tool_resources(std::move(other.tool_resources)),
        metadata(std::move(other.metadata)),
        temperature{std::move(other.temperature)},
        top_p{std::move(other.top_p)},
        response_format{std::move(other.response_format)} {}

  Assistant& operator=(Assistant&& other) noexcept {
    if (this != &other) {
      id = std::move(other.id);
      object = std::move(other.object);
      created_at = other.created_at;
      name = std::move(other.name);
      description = std::move(other.description);
      model = std::move(other.model);
      instructions = std::move(other.instructions);
      tools = std::move(other.tools);
      tool_resources = std::move(other.tool_resources);
      metadata = std::move(other.metadata);
      temperature = std::move(other.temperature);
      top_p = std::move(other.top_p);
      response_format = std::move(other.response_format);
    }
    return *this;
  }

  /**
   * The identifier, which can be referenced in API endpoints.
   */
  std::string id;

  /**
   * The object type, which is always assistant.
   */
  std::string object = "assistant";

  /**
   * The Unix timestamp (in seconds) for when the assistant was created.
   */
  uint64_t created_at;

  /**
   * The name of the assistant. The maximum length is 256 characters.
   */
  std::optional<std::string> name;

  /**
   * The description of the assistant. The maximum length is 512 characters.
   */
  std::optional<std::string> description;

  /**
   * ID of the model to use. You can use the List models API to see all of
   * your available models, or see our Model overview for descriptions of them.
   */
  std::string model;

  /**
   * The system instructions that the assistant uses. The maximum length is
   * 256,000 characters.
   */
  std::optional<std::string> instructions;

  /**
   * A list of tool enabled on the assistant. There can be a maximum of 128
   * tools per assistant. Tools can be of types code_interpreter, file_search,
   * or function.
   */
  std::vector<std::unique_ptr<AssistantTool>> tools;

  /**
   * A set of resources that are used by the assistant's tools. The resources
   * are specific to the type of tool. For example, the code_interpreter tool
   * requires a list of file IDs, while the file_search tool requires a list
   * of vector store IDs.
   */
  std::unique_ptr<OpenAi::ToolResources> tool_resources;

  /**
   * Set of 16 key-value pairs that can be attached to an object. This can be
   * useful for storing additional information about the object in a structured
   * format. Keys can be a maximum of 64 characters long and values can be a
   * maximum of 512 characters long.
   */
  Cortex::VariantMap metadata;

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

  std::variant<std::string, Json::Value> response_format;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value root;

      root["id"] = std::move(id);
      root["object"] = "assistant";
      root["created_at"] = created_at;
      if (name.has_value()) {
        root["name"] = name.value();
      }
      if (description.has_value()) {
        root["description"] = description.value();
      }
      root["model"] = model;
      if (instructions.has_value()) {
        root["instructions"] = instructions.value();
      }

      Json::Value tools_jarr{Json::arrayValue};
      for (auto& tool_ptr : tools) {
        if (auto it = tool_ptr->ToJson(); it.has_value()) {
          tools_jarr.append(it.value());
        } else {
          CTL_WRN("Failed to convert content to json: " + it.error());
        }
      }
      root["tools"] = tools_jarr;
      if (tool_resources) {
        Json::Value tool_resources_json{Json::objectValue};

        if (auto* code_interpreter =
                dynamic_cast<OpenAi::CodeInterpreter*>(tool_resources.get())) {
          auto result = code_interpreter->ToJson();
          if (result.has_value()) {
            tool_resources_json["code_interpreter"] = result.value();
          } else {
            CTL_WRN("Failed to convert code_interpreter to json: " +
                    result.error());
          }
        } else if (auto* file_search = dynamic_cast<OpenAi::FileSearch*>(
                       tool_resources.get())) {
          auto result = file_search->ToJson();
          if (result.has_value()) {
            tool_resources_json["file_search"] = result.value();
          } else {
            CTL_WRN("Failed to convert file_search to json: " + result.error());
          }
        }

        // Only add tool_resources to root if we successfully serialized some resources
        if (!tool_resources_json.empty()) {
          root["tool_resources"] = tool_resources_json;
        }
      }
      Json::Value metadata_json{Json::objectValue};
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

      if (temperature.has_value()) {
        root["temperature"] = temperature.value();
      }
      if (top_p.has_value()) {
        root["top_p"] = top_p.value();
      }
      return root;
    } catch (const std::exception& e) {
      return cpp::fail("ToJson failed: " + std::string(e.what()));
    }
  }

  static cpp::result<Assistant, std::string> FromJson(Json::Value&& json) {
    try {
      Assistant assistant;

      // Parse required fields
      if (!json.isMember("id") || !json["id"].isString()) {
        return cpp::fail("Missing or invalid 'id' field");
      }
      assistant.id = json["id"].asString();

      if (!json.isMember("object") || !json["object"].isString() ||
          json["object"].asString() != "assistant") {
        return cpp::fail("Missing or invalid 'object' field");
      }

      if (!json.isMember("created_at") || !json["created_at"].isUInt64()) {
        return cpp::fail("Missing or invalid 'created_at' field");
      }
      assistant.created_at = json["created_at"].asUInt64();

      if (!json.isMember("model") || !json["model"].isString()) {
        return cpp::fail("Missing or invalid 'model' field");
      }
      assistant.model = json["model"].asString();

      // Parse optional fields
      if (json.isMember("name") && json["name"].isString()) {
        assistant.name = json["name"].asString();
      }

      if (json.isMember("description") && json["description"].isString()) {
        assistant.description = json["description"].asString();
      }

      if (json.isMember("instructions") && json["instructions"].isString()) {
        assistant.instructions = json["instructions"].asString();
      }

      // Parse tools array
      if (json.isMember("tools") && json["tools"].isArray()) {
        auto tools_array = json["tools"];
        for (const auto& tool : tools_array) {
          if (!tool.isMember("type") || !tool["type"].isString()) {
            CTL_WRN("Tool missing type field or invalid type");
            continue;
          }

          std::string tool_type = tool["type"].asString();
          if (tool_type == "file_search") {
            auto result = AssistantFileSearchTool::FromJson(tool);
            if (result.has_value()) {
              assistant.tools.push_back(
                  std::make_unique<AssistantFileSearchTool>(
                      std::move(result.value())));
            } else {
              CTL_WRN("Failed to parse file_search tool: " + result.error());
            }
          } else if (tool_type == "code_interpreter") {
            auto result = AssistantCodeInterpreterTool::FromJson();
            if (result.has_value()) {
              assistant.tools.push_back(
                  std::make_unique<AssistantCodeInterpreterTool>(
                      std::move(result.value())));
            } else {
              CTL_WRN("Failed to parse code_interpreter tool: " +
                      result.error());
            }
          } else if (tool_type == "function") {
            auto result = AssistantFunctionTool::FromJson(tool);
            if (result.has_value()) {
              assistant.tools.push_back(std::make_unique<AssistantFunctionTool>(
                  std::move(result.value())));
            } else {
              CTL_WRN("Failed to parse function tool: " + result.error());
            }
          } else {
            CTL_WRN("Unknown tool type: " + tool_type);
          }
        }
      }

      if (json.isMember("tool_resources") &&
          json["tool_resources"].isObject()) {
        const auto& tool_resources_json = json["tool_resources"];

        // Parse code interpreter resources
        if (tool_resources_json.isMember("code_interpreter")) {
          auto result = OpenAi::CodeInterpreter::FromJson(
              tool_resources_json["code_interpreter"]);
          if (result.has_value()) {
            assistant.tool_resources =
                std::make_unique<OpenAi::CodeInterpreter>(
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
            assistant.tool_resources =
                std::make_unique<OpenAi::FileSearch>(std::move(result.value()));
          } else {
            CTL_WRN("Failed to parse file_search resources: " + result.error());
          }
        }
      }

      // Parse metadata
      if (json.isMember("metadata") && json["metadata"].isObject()) {
        auto res = Cortex::ConvertJsonValueToMap(json["metadata"]);
        if (res.has_value()) {
          assistant.metadata = res.value();
        } else {
          CTL_WRN("Failed to convert metadata to map: " + res.error());
        }
      }

      if (json.isMember("temperature") && json["temperature"].isDouble()) {
        assistant.temperature = json["temperature"].asFloat();
      }

      if (json.isMember("top_p") && json["top_p"].isDouble()) {
        assistant.top_p = json["top_p"].asFloat();
      }

      return assistant;
    } catch (const std::exception& e) {
      return cpp::fail("FromJson failed: " + std::string(e.what()));
    }
  }
};
}  // namespace OpenAi
