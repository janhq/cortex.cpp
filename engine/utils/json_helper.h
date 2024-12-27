#pragma once

#include <json/json.h>
#include <string>

namespace json_helper {
inline Json::Value ParseJsonString(const std::string& json_str) {
  Json::Value root;
  Json::Reader reader;
  reader.parse(json_str, root);
  return root;
}

inline std::string DumpJsonString(const Json::Value& json) {
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";
  return Json::writeString(builder, json);
}

inline void MergeJson(Json::Value& target, const Json::Value& source) {
  for (const auto& member : source.getMemberNames()) {
    if (target.isMember(member)) {
      // If the member exists in both objects, recursively merge the values
      if (target[member].type() == Json::objectValue &&
          source[member].type() == Json::objectValue) {
        MergeJson(target[member], source[member]);
      } else if (target[member].type() == Json::arrayValue &&
                 source[member].type() == Json::arrayValue) {
        // If the member is an array in both objects, merge the arrays
        for (const auto& value : source[member]) {
          target[member].append(value);
        }
      } else {
        // Otherwise, overwrite the value in the target with the value from the source
        target[member] = source[member];
      }
    } else {
      // If the member doesn't exist in the target, add it
      target[member] = source[member];
    }
  }
}
}  // namespace json_helper
