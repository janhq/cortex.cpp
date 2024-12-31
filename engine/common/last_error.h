#pragma once

#include <json/reader.h>
#include <string>
#include "common/json_serializable.h"

namespace OpenAi {

enum class LastErrorType { SERVER_ERROR, RATE_LIMIT_EXCEEDED, INVALID_PROMPT };

inline std::string LastErrorTypeToString(LastErrorType error_type) {
  switch (error_type) {
    case LastErrorType::SERVER_ERROR:
      return "server_error";
    case LastErrorType::RATE_LIMIT_EXCEEDED:
      return "rate_limit_exceeded";
    case LastErrorType::INVALID_PROMPT:
      return "invalid_prompt";
    default:
      return "unknown error type: #" +
             std::to_string(static_cast<int>(error_type));
  }
}

inline LastErrorType LastErrorTypeFromString(const std::string& input) {
  if (input == "server_error") {
    return LastErrorType::SERVER_ERROR;
  } else if (input == "rate_limit_exceeded") {
    return LastErrorType::RATE_LIMIT_EXCEEDED;
  } else if (input == "invalid_prompt") {
    return LastErrorType::INVALID_PROMPT;
  } else {
    return LastErrorType::SERVER_ERROR;
  }
}

struct LastError : public JsonSerializable {
  LastErrorType code;

  std::string message;

  static cpp::result<LastError, std::string> FromJsonString(
      std::string&& json_str) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json_str, root)) {
      return cpp::fail("Failed to parse JSON: " +
                       reader.getFormattedErrorMessages());
    }

    LastError last_error;

    try {
      last_error.message = std::move(root.get("message", "").asString());
      last_error.code =
          LastErrorTypeFromString(std::move(root.get("code", "").asString()));

      return last_error;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJsonString failed: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() const override {
    Json::Value root;
    root["code"] = static_cast<int>(code);
    root["message"] = message;
    return root;
  }
};
}  // namespace OpenAi
