#pragma once

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <cstdint>
#include <string>
#include "common/message_attachment.h"
#include "common/message_attachment_factory.h"
#include "common/message_content.h"
#include "common/message_content_factory.h"
#include "common/message_incomplete_detail.h"
#include "common/message_role.h"
#include "common/message_status.h"
#include "common/variant_map.h"
#include "json_serializable.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"

namespace OpenAi {

inline std::string ExtractFileId(const std::string& path) {
  // Handle both forward and backward slashes
  auto last_slash = path.find_last_of("/\\");
  if (last_slash == std::string::npos)
    return "";

  auto filename = path.substr(last_slash + 1);
  auto dot_pos = filename.find('.');
  if (dot_pos == std::string::npos)
    return "";

  return filename.substr(0, dot_pos);
}

// Represents a message within a thread.
struct Message : JsonSerializable {
  Message() = default;
  // Delete copy operations
  Message(const Message&) = delete;
  Message& operator=(const Message&) = delete;
  // Allow move operations
  Message(Message&&) = default;
  Message& operator=(Message&&) = default;

  // The identifier, which can be referenced in API endpoints.
  std::string id;

  // The object type, which is always thread.message.
  std::string object = "thread.message";

  // The Unix timestamp (in seconds) for when the message was created.
  uint32_t created_at;

  // The thread ID that this message belongs to.
  std::string thread_id;

  // The status of the message, which can be either in_progress, incomplete, or completed.
  Status status;

  // On an incomplete message, details about why the message is incomplete.
  std::optional<IncompleteDetail> incomplete_details;

  // The Unix timestamp (in seconds) for when the message was completed.
  std::optional<uint32_t> completed_at;

  // The Unix timestamp (in seconds) for when the message was marked as incomplete.
  std::optional<uint32_t> incomplete_at;

  Role role;

  // The content of the message in array of text and/or images.
  std::vector<std::unique_ptr<Content>> content;

  // If applicable, the ID of the assistant that authored this message.
  std::optional<std::string> assistant_id;

  // The ID of the run associated with the creation of this message. Value is null when messages are created manually using the create message or create thread endpoints.
  std::optional<std::string> run_id;

  // A list of files attached to the message, and the tools they were added to.
  std::optional<std::vector<Attachment>> attachments;

  // Set of 16 key-value pairs that can be attached to an object. This can be useful for storing additional information about the object in a structured format. Keys can be a maximum of 64 characters long and values can be a maximum of 512 characters long.
  Cortex::VariantMap metadata;

  // deprecated. remove in the future
  std::optional<std::string> attach_filename;
  std::optional<uint64_t> size;
  std::optional<std::string> rel_path;
  // end deprecated

  static cpp::result<Message, std::string> FromJsonString(
      std::string&& json_str) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json_str, root)) {
      return cpp::fail("Failed to parse JSON: " +
                       reader.getFormattedErrorMessages());
    }

    Message message;

    try {
      message.id = std::move(root["id"].asString());
      message.object =
          std::move(root.get("object", "thread.message").asString());
      message.created_at = root["created_at"].asUInt();
      if (message.created_at == 0 && root["created"].asUInt64() != 0) {
        message.created_at =
            static_cast<uint32_t>(root["created"].asUInt64() / 1000);
      }
      message.thread_id = std::move(root["thread_id"].asString());
      message.status = StatusFromString(std::move(root["status"].asString()));

      message.incomplete_details =
          IncompleteDetail::FromJson(std::move(root["incomplete_details"]))
              .value();
      message.completed_at = root["completed_at"].asUInt();
      message.incomplete_at = root["incomplete_at"].asUInt();
      message.role = RoleFromString(std::move(root["role"].asString()));

      message.assistant_id = std::move(root["assistant_id"].asString());
      message.run_id = std::move(root["run_id"].asString());
      message.attachments =
          ParseAttachments(std::move(root["attachments"])).value();

      if (root["metadata"].isObject() && !root["metadata"].empty()) {
        auto res = Cortex::ConvertJsonValueToMap(root["metadata"]);
        if (res.has_error()) {
          CTL_WRN("Failed to convert metadata to map: " + res.error());
        } else {
          message.metadata = res.value();
        }
      }

      if (root.isMember("content")) {
        if (root["content"].isArray() && !root["content"].empty()) {
          if (root["content"][0]["type"].asString() == "text") {
            message.content = ParseContents(std::move(root["content"])).value();
          } else if (root["content"][0]["type"].asString() == "image") {
            // deprecated, for supporting jan and should be removed in the future
            auto text_str = root["content"][0]["text"]["value"].asString();
            auto img_url =
                root["content"][0]["text"]["annotations"][0].asString();
            auto text_content = std::make_unique<OpenAi::TextContent>();
            {
              auto text = OpenAi::Text();
              auto empty_annotations =
                  std::vector<std::unique_ptr<Annotation>>();
              text.value = std::move(text_str);
              text.annotations = std::move(empty_annotations);
              text_content->text = std::move(text);
            }

            auto image_url_obj = OpenAi::ImageUrl(img_url, "auto");
            auto image_url_content = std::make_unique<OpenAi::ImageUrlContent>(
                "image_url", std::move(image_url_obj));

            message.content.push_back(std::move(text_content));
            message.content.push_back(std::move(image_url_content));
          } else {
            // deprecated, for supporting jan and should be removed in the future
            // check if annotations is empty
            if (!root["content"][0]["text"]["annotations"].empty()) {
              // parse attachment
              Json::Value attachments_json_array{Json::arrayValue};
              Json::Value attachment;
              attachment["file_id"] = ExtractFileId(
                  root["content"][0]["text"]["annotations"][0].asString());

              Json::Value tools_json_array{Json::arrayValue};
              Json::Value tool;
              tool["type"] = "file_search";
              tools_json_array.append(tool);

              attachment["tools"] = tools_json_array;
              attachment["file_id"] = attachments_json_array.append(attachment);

              message.attachments =
                  ParseAttachments(std::move(attachments_json_array)).value();

              message.attach_filename =
                  root["content"][0]["text"]["name"].asString();
              message.size = root["content"][0]["text"]["size"].asUInt64();
              message.rel_path =
                  root["content"][0]["text"]["annotations"][0].asString();
            }

            // parse content
            Json::Value contents_json_array{Json::arrayValue};
            Json::Value content;
            Json::Value content_text;
            Json::Value empty_annotations{Json::arrayValue};
            content["type"] = "text";
            content_text["value"] = root["content"][0]["text"]["value"];
            content_text["annotations"] = empty_annotations;
            content["text"] = content_text;
            contents_json_array.append(content);
            message.content =
                ParseContents(std::move(contents_json_array)).value();
          }
        }
      }

      return message;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJsonString failed: ") + e.what());
    }
  }

  cpp::result<std::string, std::string> ToSingleLineJsonString() {
    auto json_result = ToJson();
    if (json_result.has_error()) {
      return cpp::fail(json_result.error());
    }

    Json::FastWriter writer;
    try {
      return writer.write(json_result.value());
    } catch (const std::exception& e) {
      return cpp::fail(std::string("Failed to write JSON: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;

      json["id"] = id;
      json["object"] = object;
      json["created_at"] = created_at;
      json["thread_id"] = thread_id;
      json["status"] = StatusToString(status);

      if (incomplete_details.has_value()) {
        if (auto it = incomplete_details->ToJson(); it.has_value()) {
          json["incomplete_details"] = it.value();
        } else {
          CTL_WRN("Failed to convert incomplete_details to json: " +
                  it.error());
        }
      }
      if (completed_at.has_value() && completed_at.value() != 0) {
        json["completed_at"] = *completed_at;
      }
      if (incomplete_at.has_value() && incomplete_at.value() != 0) {
        json["incomplete_at"] = *incomplete_at;
      }

      json["role"] = RoleToString(role);

      Json::Value content_json_arr{Json::arrayValue};
      for (auto& child_content : content) {
        if (auto it = child_content->ToJson(); it.has_value()) {
          content_json_arr.append(it.value());
        } else {
          CTL_WRN("Failed to convert content to json: " + it.error());
        }
      }
      json["content"] = content_json_arr;
      if (assistant_id.has_value() && !assistant_id->empty()) {
        json["assistant_id"] = *assistant_id;
      }
      if (run_id.has_value() && !run_id->empty()) {
        json["run_id"] = *run_id;
      }
      if (attachments.has_value()) {
        Json::Value attachments_json_arr{Json::arrayValue};
        for (auto& attachment : *attachments) {
          if (auto it = attachment.ToJson(); it.has_value()) {
            attachments_json_arr.append(it.value());
          } else {
            CTL_WRN("Failed to convert attachment to json: " + it.error());
          }
        }
        json["attachments"] = attachments_json_arr;
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

      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
};  // namespace OpenAi
