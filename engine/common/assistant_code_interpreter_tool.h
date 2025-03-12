#pragma once

#include "common/assistant_tool.h"

namespace OpenAi {
struct AssistantCodeInterpreterTool : public AssistantTool {
  AssistantCodeInterpreterTool() : AssistantTool("code_interpreter") {}

  AssistantCodeInterpreterTool(const AssistantCodeInterpreterTool&) = delete;

  AssistantCodeInterpreterTool& operator=(const AssistantCodeInterpreterTool&) =
      delete;

  AssistantCodeInterpreterTool(AssistantCodeInterpreterTool&&) = default;

  AssistantCodeInterpreterTool& operator=(AssistantCodeInterpreterTool&&) =
      default;

  ~AssistantCodeInterpreterTool() = default;

  static cpp::result<AssistantCodeInterpreterTool, std::string> FromJson() {
    AssistantCodeInterpreterTool tool;
    return tool;
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value json;
    json["type"] = type;
    return json;
  }
};
}  // namespace OpenAi
