#pragma once

#include <json/value.h>
#include <string>
#include <unordered_map>
#include <variant>
#include "utils/result.hpp"

namespace Cortex {

using ValueVariant = std::variant<std::string, bool, uint64_t, double>;
using VariantMap = std::unordered_map<std::string, ValueVariant>;

inline cpp::result<VariantMap, std::string> ConvertJsonValueToMap(
    const Json::Value& json) {
  VariantMap result;

  if (!json.isObject()) {
    return cpp::fail("Input json is not an object");
  }

  for (const auto& key : json.getMemberNames()) {
    const Json::Value& value = json[key];

    switch (value.type()) {
      case Json::nullValue:
        // Skip null values
        break;

      case Json::stringValue:
        result.emplace(key, value.asString());
        break;

      case Json::booleanValue:
        result.emplace(key, value.asBool());
        break;

      case Json::uintValue:
      case Json::intValue:
        // Handle both signed and unsigned integers
        if (value.isUInt64()) {
          result.emplace(key, value.asUInt64());
        } else {
          // Convert to double if the integer is negative or too large
          result.emplace(key, value.asDouble());
        }
        break;

      case Json::realValue:
        result.emplace(key, value.asDouble());
        break;

      case Json::arrayValue:
      case Json::objectValue:
        // currently does not handle complex type
        break;
    }
  }

  return result;
}
};  // namespace Cortex
