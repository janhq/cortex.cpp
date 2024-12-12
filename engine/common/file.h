#pragma once

#include <string>
#include "common/json_serializable.h"

namespace OpenAi {
/**
 * The File object represents a document that has been uploaded to OpenAI.
 */
struct File : public JsonSerializable {
  /**
   * The file identifier, which can be referenced in the API endpoints.
   */
  std::string id;

  /**
   * The object type, which is always file.
   */
  std::string object = "file";

  /**
   * The size of the file, in bytes.
   */
  uint64_t bytes;

  /**
   * The Unix timestamp (in seconds) for when the file was created.
   */
  uint32_t created_at;

  /**
   * The name of the file.
   */
  std::string filename;

  /**
   * The intended purpose of the file. Supported values are assistants,
   * assistants_output, batch, batch_output, fine-tune, fine-tune-results
   * and vision.
   */
  std::string purpose;

  ~File() = default;

  static cpp::result<File, std::string> FromJson(const Json::Value& json) {
    File file;

    file.id = std::move(json["id"].asString());
    file.object = "file";
    file.bytes = json["bytes"].asUInt64();
    file.created_at = json["created_at"].asUInt();
    file.filename = std::move(json["filename"].asString());
    file.purpose = std::move(json["purpose"].asString());

    return file;
  }

  cpp::result<Json::Value, std::string> ToJson() {
    Json::Value root;

    root["id"] = id;
    root["object"] = object;
    root["bytes"] = bytes;
    root["created_at"] = created_at;
    root["filename"] = filename;
    root["purpose"] = purpose;

    return root;
  }
};
}  // namespace OpenAi
