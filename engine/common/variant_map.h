#pragma once

#include <json/reader.h>
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

inline cpp::result<VariantMap, std::string> VariantMapFromJsonString(
    std::string&& json_str) {
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(json_str, root)) {
    return cpp::fail("Failed to parse JSON: " +
                     reader.getFormattedErrorMessages());
  }
  return ConvertJsonValueToMap(root);
}

inline cpp::result<std::string, std::string> VariantMapToString(
    const VariantMap& map) {
  Json::Value root(Json::objectValue);

  for (const auto& [key, value] : map) {
    std::visit(
        [&root, &key](auto&& arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::string>) {
            root[key] = arg;
          } else if constexpr (std::is_same_v<T, bool>) {
            root[key] = arg;
          } else if constexpr (std::is_same_v<T, uint64_t>) {
            root[key] = arg;
          } else if constexpr (std::is_same_v<T, double>) {
            root[key] = arg;
          }
        },
        value);
  }

  return root.toStyledString();
}
};  // namespace Cortex
