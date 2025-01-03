#pragma once

#include <optional>
#include "common/assistant_tool.h"
#include "common/json_serializable.h"

namespace OpenAi {
struct AssistantFunction : public JsonSerializable {
  AssistantFunction(const std::string& description, const std::string& name,
                    const Json::Value& parameters,
                    const std::optional<bool>& strict)
      : description{std::move(description)},
        name{std::move(name)},
        parameters{std::move(parameters)},
        strict{strict} {}

  AssistantFunction(const AssistantFunction&) = delete;

  AssistantFunction& operator=(const AssistantFunction&) = delete;

  AssistantFunction(AssistantFunction&&) = default;

  AssistantFunction& operator=(AssistantFunction&&) = default;

  ~AssistantFunction() = default;

  /**
   * A description of what the function does, used by the model to choose
   * when and how to call the function.
   */
  std::string description;

  /**
   * The name of the function to be called. Must be a-z, A-Z, 0-9, or contain
   * underscores and dashes, with a maximum length of 64.
   */
  std::string name;

  /**
   * The parameters the functions accepts, described as a JSON Schema object.
   * See the guide for examples, and the JSON Schema reference for documentation
   * about the format.
   *
   * Omitting parameters defines a function with an empty parameter list.
   */
  Json::Value parameters;

  /**
   * Whether to enable strict schema adherence when generating the function call.
   * If set to true, the model will follow the exact schema defined in the parameters
   * field. Only a subset of JSON Schema is supported when strict is true.
   *
   * Learn more about Structured Outputs in the function calling guide.
   */
  std::optional<bool> strict;

  static cpp::result<AssistantFunction, std::string> FromJson(
      const Json::Value& json) {
    if (json.empty()) {
      return cpp::fail("Function json can't be empty");
    }

    if (!json.isMember("name") || json.get("name", "").asString().empty()) {
      return cpp::fail("Function name can't be empty");
    }

    if (!json.isMember("description")) {
      return cpp::fail("Function description is mandatory");
    }

    if (!json.isMember("parameters")) {
      return cpp::fail("Function parameters are mandatory");
    }

    std::optional<bool> is_strict = std::nullopt;
    if (json.isMember("strict")) {
      is_strict = json["strict"].asBool();
    }
    AssistantFunction function{json["description"].asString(),
                               json["name"].asString(), json["parameters"],
                               is_strict};
    function.parameters = json["parameters"];
    return function;
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value json;
    json["description"] = description;
    json["name"] = name;
    if (strict.has_value()) {
      json["strict"] = *strict;
    }
    json["parameters"] = parameters;
    return json;
  }
};

struct AssistantFunctionTool : public AssistantTool {
  AssistantFunctionTool(AssistantFunction& function)
      : AssistantTool("function"), function{std::move(function)} {}

  AssistantFunctionTool(const AssistantFunctionTool&) = delete;

  AssistantFunctionTool& operator=(const AssistantFunctionTool&) = delete;

  AssistantFunctionTool(AssistantFunctionTool&&) = default;

  AssistantFunctionTool& operator=(AssistantFunctionTool&&) = default;

  ~AssistantFunctionTool() = default;

  AssistantFunction function;

  static cpp::result<AssistantFunctionTool, std::string> FromJson(
      const Json::Value& json) {
    auto function_res = AssistantFunction::FromJson(json["function"]);
    if (function_res.has_error()) {
      return cpp::fail("Failed to parse function: " + function_res.error());
    }
    return AssistantFunctionTool{function_res.value()};
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value root;
    root["type"] = type;
    root["function"] = function.ToJson().value();
    return root;
  }
};
};  // namespace OpenAi
