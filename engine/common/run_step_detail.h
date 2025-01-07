#pragma once

#include <string>
#include "common/json_serializable.h"

namespace OpenAi {
struct RunStepDetails : public JsonSerializable {
  RunStepDetails(const std::string& type) : type{type} {}

  virtual ~RunStepDetails() = default;

  std::string type;
};

struct MessageCreationDetail : public RunStepDetails {
  struct MessageCreation {
    std::string message_id;
  };

  MessageCreationDetail() : RunStepDetails{"message_creation"} {}

  ~MessageCreationDetail() = default;

  MessageCreation message_creation;

  cpp::result<Json::Value, std::string> ToJson() const {
    Json::Value root;
    root["type"] = type;
    root["message_creation"]["message_id"] = message_creation.message_id;
    return root;
  }
};

struct ToolCalls : public RunStepDetails {
  ToolCalls() : RunStepDetails{"tool_calls"} {}

  ~ToolCalls() = default;
};
}  // namespace OpenAi
