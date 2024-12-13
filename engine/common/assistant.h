#pragma once

#include <string>
#include "common/assistant_tool.h"
#include "common/thread_tool_resources.h"
#include "common/variant_map.h"
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

struct Assistant {
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
  std::optional<std::variant<ThreadCodeInterpreter, ThreadFileSearch>>
      tool_resources;

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
};
}  // namespace OpenAi
