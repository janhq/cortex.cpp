#pragma once

#include <json/value.h>
#include <string>
#include "common/json_serializable.h"
#include "utils/result.hpp"

namespace OpenAi {
struct ToolChoice : public JsonSerializable {
  struct Function {
    /**
     * The name of the function to call.
     */
    std::string name;
  };

  /**
   * The type of the tool. Currently, only function is supported.
   */
  std::string type;

  /**
   * Specifies a tool the model should use. Use to force the model
   * to call a specific function.
   */
  Function function;

  static cpp::result<ToolChoice, std::string> FromJson(Json::Value&& json) {
    try {
      ToolChoice tool_choice;
      if (json.isMember("type") && json["type"].isString()) {
        tool_choice.type = json["type"].asString();
      } else {
        return cpp::fail("Missing or invalid 'type' field");
      }
      if (json.isMember("function") && json["function"].isObject()) {
        auto function = json["function"];
        if (function.isMember("name") && function["name"].isString()) {
          tool_choice.function.name = function["name"].asString();
        } else {
          return cpp::fail("Missing or invalid 'name' field in 'function'");
        }
      } else {
        return cpp::fail("Missing or invalid 'function' field");
      }
      return tool_choice;
    } catch (const std::exception& e) {
      return cpp::fail("FromJson failed: " + std::string(e.what()));
    }
  }

  cpp::result<Json::Value, std::string> ToJson() const {
    Json::Value json;
    json["type"] = type;
    json["function"]["name"] = function.name;
    return json;
  }
};
}  // namespace OpenAi
