#include "common/assistant_tool.h"

namespace OpenAi {
class AssistantCodeInterpreterTool : public AssistantTool {
  AssistantCodeInterpreterTool() : AssistantTool("code_interpreter") {}

  AssistantCodeInterpreterTool(AssistantCodeInterpreterTool&) = delete;

  AssistantCodeInterpreterTool& operator=(AssistantCodeInterpreterTool&) =
      delete;

  AssistantCodeInterpreterTool(AssistantCodeInterpreterTool&&) = default;

  AssistantCodeInterpreterTool& operator=(AssistantCodeInterpreterTool&&) =
      default;

  ~AssistantCodeInterpreterTool() = default;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["type"] = type;
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
}  // namespace OpenAi
