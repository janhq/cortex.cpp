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
}