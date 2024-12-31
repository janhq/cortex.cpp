#pragma once

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

struct LastError : public JsonSerializable {
  LastErrorType code;

  std::string message;

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value root;
    root["code"] = static_cast<int>(code);
    root["message"] = message;
    return root;
  }
};
}  // namespace OpenAi
