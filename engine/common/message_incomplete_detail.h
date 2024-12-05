#pragma once

#include "common/json_serializable.h"

namespace OpenAi {

// On an incomplete message, details about why the message is incomplete.
struct IncompleteDetail : JsonSerializable {
  // The reason the message is incomplete.
  std::string reason;

  static cpp::result<std::optional<IncompleteDetail>, std::string> FromJson(
      Json::Value&& json) {
    if (json.empty()) {
      return std::nullopt;
    }
    IncompleteDetail incomplete_detail;
    incomplete_detail.reason = json["reason"].asString();
    return incomplete_detail;
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["reason"] = reason;
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
}  // namespace OpenAi
