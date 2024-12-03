#pragma once

#include "common/json_serializable.h"

namespace api_response {
struct DeleteMessageResponse : JsonSerializable {
  std::string id;
  std::string object;
  bool deleted;

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value json;
    json["id"] = id;
    json["object"] = object;
    json["deleted"] = deleted;
    return json;
  }
};
}  // namespace api_response
