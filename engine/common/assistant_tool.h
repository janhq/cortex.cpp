#pragma once

#include <string>
#include "common/json_serializable.h"

namespace OpenAi {
struct AssistantTool : public JsonSerializable {
  std::string type;

  AssistantTool(const std::string& type) : type{type} {}

  AssistantTool(const AssistantTool&) = delete;

  AssistantTool& operator=(const AssistantTool&) = delete;

  AssistantTool(AssistantTool&& other) noexcept : type{std::move(other.type)} {}

  AssistantTool& operator=(AssistantTool&& other) noexcept {
    if (this != &other) {
      type = std::move(other.type);
    }
    return *this;
  }

  virtual ~AssistantTool() = default;
};
}  // namespace OpenAi
