#pragma once

#include <json/writer.h>
#include "common/events/assistant_stream_event.h"
#include "utils/result.hpp"

namespace OpenAi {
struct ErrorEvent : public AssistantStreamEvent {
  ErrorEvent(const std::string& error)
      : error{std::move(error)}, AssistantStreamEvent("error") {}

  ~ErrorEvent() = default;

  std::string error;

  auto SingleLineJsonData() const
      -> cpp::result<std::string, std::string> override {
    Json::Value json;
    json["error"] = error;
    Json::FastWriter writer;
    try {
      std::string json_str = writer.write(json);
      if (!json_str.empty() && json_str.back() == '\n') {
        json_str.pop_back();
      }
      return json_str;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("Failed to write JSON: ") + e.what());
    }
  }
};
}  // namespace OpenAi
