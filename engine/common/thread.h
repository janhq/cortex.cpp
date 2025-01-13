#pragma once

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include "common/assistant.h"
#include "common/tool_resources.h"
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
  std::unique_ptr<ToolResources> tool_resources;

  /**
   * Set of 16 key-value pairs that can be attached to an object.
   * This can be useful for storing additional information about the object 
   * in a structured format. 
   * 
   * Keys can be a maximum of 64 characters long and values can be a maximum
   * of 512 characters long.
   */
  Cortex::VariantMap metadata;

  // For supporting Jan
  std::optional<std::vector<JanAssistant>> assistants;

  static cpp::result<Thread, std::string> FromJson(const Json::Value& json) {
    Thread thread;

    thread.id = json["id"].asString();
    thread.object = "thread";
    thread.created_at = json["created_at"].asUInt();
    if (thread.created_at == 0 && json["created"].asUInt64() != 0) {
      thread.created_at = json["created"].asUInt64() / 1000;
    }

    if (json.isMember("tool_resources") && !json["tool_resources"].isNull()) {
      const auto& tool_json = json["tool_resources"];

      if (tool_json.isMember("code_interpreter")) {
        auto code_interpreter = std::make_unique<CodeInterpreter>();
        const auto& file_ids = tool_json["code_interpreter"]["file_ids"];
        if (file_ids.isArray()) {
          for (const auto& file_id : file_ids) {
            code_interpreter->file_ids.push_back(file_id.asString());
          }
        }
        thread.tool_resources = std::move(code_interpreter);
      } else if (tool_json.isMember("file_search")) {
        auto file_search = std::make_unique<FileSearch>();
        const auto& store_ids = tool_json["file_search"]["vector_store_ids"];
        if (store_ids.isArray()) {
          for (const auto& store_id : store_ids) {
            file_search->vector_store_ids.push_back(store_id.asString());
          }
        }
        thread.tool_resources = std::move(file_search);
      }
    }

    if (json["metadata"].isObject() && !json["metadata"].empty()) {
      auto res = Cortex::ConvertJsonValueToMap(json["metadata"]);
      if (res.has_error()) {
        CTL_WRN("Failed to convert metadata to map: " + res.error());
      } else {
        thread.metadata = res.value();
      }
    }

    if (json.isMember("title") && !json["title"].isNull()) {
      thread.metadata["title"] = json["title"].asString();
    }

    if (json.isMember("assistants") && json["assistants"].isArray()) {
      std::vector<JanAssistant> assistants;
      for (Json::ArrayIndex i = 0; i < json["assistants"].size(); ++i) {
        Json::Value assistant_json = json["assistants"][i];
        auto assistant_result =
            JanAssistant::FromJson(std::move(assistant_json));
        if (assistant_result.has_error()) {
          return cpp::fail("Failed to parse assistant: " +
                           assistant_result.error());
        }
        assistants.push_back(std::move(assistant_result.value()));
      }
      thread.assistants = std::move(assistants);
    }

    return thread;
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;

      json["id"] = id;
      json["object"] = object;
      json["created_at"] = created_at;

      // Deprecated: This is for backward compatibility. Please remove it later. (2-3 releases) to be sure
      try {
        auto it = metadata.find("title");
        if (it == metadata.end()) {
          json["title"] = "";
        } else {
          json["title"] = std::get<std::string>(metadata["title"]);
        }

      } catch (const std::bad_variant_access& ex) {
        // std::cerr << "Error: value is not a string" << std::endl;
        CTL_WRN("Error: value of title is not a string: " << ex.what());
      }
      // End deprecated

      if (tool_resources) {
        auto tool_result = tool_resources->ToJson();
        if (tool_result.has_error()) {
          return cpp::fail("Failed to serialize tool_resources: " +
                           tool_result.error());
        }

        Json::Value tool_json;
        if (auto code_interpreter =
                dynamic_cast<CodeInterpreter*>(tool_resources.get())) {
          tool_json["code_interpreter"] = tool_result.value();
        } else if (auto file_search =
                       dynamic_cast<FileSearch*>(tool_resources.get())) {
          tool_json["file_search"] = tool_result.value();
        }
        json["tool_resources"] = tool_json;
      }

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

      if (assistants.has_value()) {
        Json::Value assistants_json(Json::arrayValue);
        for (auto& assistant : assistants.value()) {
          auto assistant_result = assistant.ToJson();
          if (assistant_result.has_error()) {
            return cpp::fail("Failed to serialize assistant: " +
                             assistant_result.error());
          }
          assistants_json.append(assistant_result.value());
        }
        json["assistants"] = assistants_json;
      }

      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
}  // namespace OpenAi
