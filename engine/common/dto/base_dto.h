#pragma once

#include <json/value.h>
#include "utils/result.hpp"

namespace dto {
template <typename T>
struct BaseDto {
  virtual ~BaseDto() = default;

  /**
   * Validate itself.
   */
  virtual cpp::result<void, std::string> Validate() const = 0;
};
}  // namespace dto
