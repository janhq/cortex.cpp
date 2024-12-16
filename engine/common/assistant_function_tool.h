#include "common/assistant_tool.h"
#include "common/json_serializable.h"

namespace OpenAi {
struct AssistantFunction : public JsonSerializable {
  AssistantFunction(const std::string& description, const std::string& name,
                    const std::optional<bool>& strict)
      : description{description}, name{name}, strict{strict} {}

  AssistantFunction(AssistantFunction&) = delete;

  AssistantFunction& operator=(AssistantFunction&) = delete;

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

  // TODO: namh handle parameters

  /**
     * Whether to enable strict schema adherence when generating the function call.
     * If set to true, the model will follow the exact schema defined in the parameters
     * field. Only a subset of JSON Schema is supported when strict is true.
     *
     * Learn more about Structured Outputs in the function calling guide.
     */
  std::optional<bool> strict;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["description"] = description;
      json["name"] = name;
      if (strict.has_value()) {
        json["strict"] = *strict;
      }
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};

struct AssistantFunctionTool : public AssistantTool {
  AssistantFunctionTool(AssistantFunction& function)
      : AssistantTool("function"), function{std::move(function)} {}

  AssistantFunctionTool(AssistantFunctionTool&) = delete;

  AssistantFunctionTool& operator=(AssistantFunctionTool&) = delete;

  AssistantFunctionTool(AssistantFunctionTool&&) = default;

  AssistantFunctionTool& operator=(AssistantFunctionTool&&) = default;

  ~AssistantFunctionTool() = default;

  AssistantFunction function;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value root;
      root["type"] = type;
      root["function"] = function.ToJson().value();
      return root;
    } catch (const std::exception& e) {
      return cpp::fail("To Json failed: " + std::string(e.what()));
    }
  }
};
};  // namespace OpenAi
