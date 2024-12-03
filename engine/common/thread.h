#pragma once

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include "common/thread_tool_resources.h"
#include "common/variant_map.h"
#include "json_serializable.h"
#include "utils/logging_utils.h"

namespace OpenAi {

/**
 * Represents a thread that contains messages.
 */
struct Thread : JsonSerializable {
  /**
   * The identifier, which can be referenced in API endpoints.
   */
  std::string id;

  /**
   * The object type, which is always thread.
   */
  std::string object = "thread";

  /**
   * The Unix timestamp (in seconds) for when the thread was created.
   */
  uint64_t created_at;

  /**
   * A set of resources that are made available to the assistant's
   * tools in this thread. The resources are specific to the type
   * of tool. For example, the code_interpreter tool requires a list of
   * file IDs, while the file_search tool requires a list of vector store IDs.
   */
  std::optional<std::unique_ptr<ThreadToolResources>> tool_resources;

  /**
   * Set of 16 key-value pairs that can be attached to an object.
   * This can be useful for storing additional information about the object 
   * in a structured format. 
   * 
   * Keys can be a maximum of 64 characters long and values can be a maximum
   * of 512 characters long.
   */
  Cortex::VariantMap metadata;

  static cpp::result<Thread, std::string> FromJson(const Json::Value& json) {
    Thread thread;

    thread.id = json["id"].asString();
    thread.object = "thread";
    thread.created_at = json["created_at"].asUInt();
    if (thread.created_at == 0 && json["created"].asUInt64() != 0) {
      thread.created_at = json["created"].asUInt64() / 1000;
    }
    // TODO: namh parse tool_resources

    if (json["metadata"].isObject() && !json["metadata"].empty()) {
      auto res = Cortex::ConvertJsonValueToMap(json["metadata"]);
      if (res.has_error()) {
        CTL_WRN("Failed to convert metadata to map: " + res.error());
      } else {
        thread.metadata = res.value();
      }
    }

    return thread;
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;

      json["id"] = id;
      json["object"] = object;
      json["created_at"] = created_at;
      // TODO: namh handle tool_resources

      Json::Value metadata_json{Json::objectValue};
      for (const auto& [key, value] : metadata) {
        if (std::holds_alternative<bool>(value)) {
          metadata_json[key] = std::get<bool>(value);
        } else if (std::holds_alternative<uint64_t>(value)) {
          metadata_json[key] = std::get<uint64_t>(value);
        } else if (std::holds_alternative<double>(value)) {
          metadata_json[key] = std::get<double>(value);
        } else {
          metadata_json[key] = std::get<std::string>(value);
        }
      }
      json["metadata"] = metadata_json;

      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
}  // namespace OpenAi
