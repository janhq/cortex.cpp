#pragma once

#include <json/value.h>
#include <string>
#include "utils/logging_utils.h"

namespace json_parser_utils {

template <typename T>
T jsonToValue(const Json::Value& value);

template <>
std::string jsonToValue(const Json::Value& value) {
  return value.asString();
}

template <typename T>
std::vector<T> ParseJsonArray(const Json::Value& array) {
  try {
    std::vector<T> result;
    if (array.isArray()) {
      result.reserve(array.size());
      for (const Json::Value& element : array) {
        result.push_back(jsonToValue<T>(element));
      }
    }
    return result;
  } catch (const std::exception& e) {
    CTL_ERR("Error parsing json array: " << e.what());
    return {};
  }
}
};  // namespace json_parser_utils
