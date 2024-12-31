#pragma once

#include <json/reader.h>
#include <cstdint>
#include "common/json_serializable.h"

namespace OpenAi {
struct RunUsage : public JsonSerializable {

  RunUsage() = default;

  ~RunUsage() = default;

  uint64_t completion_tokens;

  uint64_t prompt_tokens;

  uint64_t total_tokens;

  static cpp::result<RunUsage, std::string> FromJsonString(
      std::string&& json_str) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json_str, root)) {
      return cpp::fail("Failed to parse JSON: " +
                       reader.getFormattedErrorMessages());
    }

    RunUsage run_usage;

    try {
      run_usage.completion_tokens = root["completion_tokens"].asUInt64();
      run_usage.prompt_tokens = root["prompt_tokens"].asUInt64();
      run_usage.total_tokens = root["total_tokens"].asUInt64();

      return run_usage;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJsonString failed: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() const {
    Json::Value json;
    json["completion_tokens"] = completion_tokens;
    json["prompt_tokens"] = prompt_tokens;
    json["total_tokens"] = total_tokens;
    return json;
  }
};
}  // namespace OpenAi
