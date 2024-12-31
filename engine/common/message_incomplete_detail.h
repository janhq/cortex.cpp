#pragma once

#include <json/reader.h>
#include "common/json_serializable.h"

namespace OpenAi {

// On an incomplete message, details about why the message is incomplete.
struct IncompleteDetail : JsonSerializable {
  IncompleteDetail(const std::string& reason) : reason{reason} {}

  /**
   * The reason the message is incomplete.
   */
  std::string reason;

  static cpp::result<IncompleteDetail, std::string> FromJsonString(
      std::string&& json_str) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json_str, root)) {
      return cpp::fail("Failed to parse JSON: " +
                       reader.getFormattedErrorMessages());
    }

    return IncompleteDetail(root.get("reason", "").asString());
  }

  static cpp::result<std::optional<IncompleteDetail>, std::string> FromJson(
      Json::Value&& json) {
    if (json.empty()) {
      return std::nullopt;
    }
    return IncompleteDetail(json["reason"].asString());
  }

  cpp::result<Json::Value, std::string> ToJson() const override {
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
