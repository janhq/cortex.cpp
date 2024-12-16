#pragma once

#include <string>
#include "common/json_serializable.h"

namespace OpenAi {
struct AssistantTool : public JsonSerializable {
  std::string type;

  AssistantTool(const std::string& type) : type{type} {}

  virtual ~AssistantTool() = default;
};
}  // namespace OpenAi
