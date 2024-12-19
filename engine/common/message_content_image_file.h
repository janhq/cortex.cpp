#pragma once

#include "common/message_content.h"

namespace OpenAi {
struct ImageFile {
  // The File ID of the image in the message content. Set purpose="vision" when uploading the File if you need to later display the file content.
  std::string file_id;

  // Specifies the detail level of the image if specified by the user. low uses fewer tokens, you can opt in to high resolution using high.
  std::string detail;

  ImageFile() = default;

  ImageFile(ImageFile&&) noexcept = default;

  ImageFile& operator=(ImageFile&&) noexcept = default;

  ImageFile(const ImageFile&) = delete;

  ImageFile& operator=(const ImageFile&) = delete;
};

// References an image File in the content of a message.
struct ImageFileContent : Content {

  ImageFileContent() : Content("image_file") {}

  ImageFileContent(ImageFileContent&&) noexcept = default;

  ImageFileContent& operator=(ImageFileContent&&) noexcept = default;

  ImageFileContent(const ImageFileContent&) = delete;

  ImageFileContent& operator=(const ImageFileContent&) = delete;

  ImageFile image_file;

  static cpp::result<ImageFileContent, std::string> FromJson(
      Json::Value&& json) {
    if (json.empty()) {
      return cpp::fail("Json string is empty");
    }

    try {
      ImageFileContent content;
      ImageFile image_file;
      image_file.detail = std::move(json["image_file"]["detail"].asString());
      image_file.file_id = std::move(json["image_file"]["file_id"].asString());
      content.image_file = std::move(image_file);
      return content;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJson failed: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["type"] = type;
      json["image_file"]["file_id"] = image_file.file_id;
      json["image_file"]["detail"] = image_file.detail;
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
}  // namespace OpenAi
