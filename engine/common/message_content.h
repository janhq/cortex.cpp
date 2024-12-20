#pragma once

#include <string>
#include "common/json_serializable.h"

namespace OpenAi {

struct Content : JsonSerializable {
  std::string type;

  Content(const std::string& type) : type{type} {}

  Content(const Content&) = delete;

  Content& operator=(const Content&) = delete;

  Content(Content&&) noexcept = default;

  Content& operator=(Content&&) noexcept = default;

  virtual ~Content() = default;
};
};  // namespace OpenAi
