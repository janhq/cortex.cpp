#pragma once

#include <string>
#include "common/dto/base_dto.h"
#include "common/variant_map.h"
#include "utils/logging_utils.h"

namespace dto {
struct RunUpdateDto : public BaseDto<RunUpdateDto> {
  RunUpdateDto() = default;

  ~RunUpdateDto() = default;

  RunUpdateDto(const RunUpdateDto&) = delete;

  RunUpdateDto& operator=(const RunUpdateDto&) = delete;

  RunUpdateDto(RunUpdateDto&& other) noexcept
      : metadata{std::move(other.metadata)} {}

  RunUpdateDto& operator=(RunUpdateDto&& other) noexcept {
    if (this != &other) {
      metadata = std::move(other.metadata);
    }
    return *this;
  }

  /**
   * Set of 16 key-value pairs that can be attached to an object.
   * This can be useful for storing additional information about the object
   * in a structured format. Keys can be a maximum of 64 characters long
   * and values can be a maximum of 512 characters long.
   */
  std::optional<Cortex::VariantMap> metadata;

  cpp::result<void, std::string> Validate() const {
    if (!metadata.has_value()) {
      return cpp::fail("Nothing to update");
    }

    return {};
  }

  static cpp::result<RunUpdateDto, std::string> FromJson(Json::Value&& json) {
    try {
      RunUpdateDto dto;

      // Parse metadata
      if (json.isMember("metadata") && json["metadata"].isObject()) {
        auto res = Cortex::ConvertJsonValueToMap(json["metadata"]);
        if (res.has_value()) {
          dto.metadata = res.value();
        } else {
          CTL_WRN("Failed to convert metadata to map: " + res.error());
        }
      }

      return dto;
    } catch (const std::exception& e) {
      return cpp::fail("FromJson failed: " + std::string(e.what()));
    }
  }
};
}  // namespace dto
