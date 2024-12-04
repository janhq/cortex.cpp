#pragma once

#include "common/message_content.h"

namespace ThreadMessage {

struct ImageUrl {
  // The external URL of the image, must be a supported image types: jpeg, jpg, png, gif, webp.
  std::string url;

  // Specifies the detail level of the image. low uses fewer tokens, you can opt in to high resolution using high. Default value is auto
  std::string detail;

  ImageUrl() = default;

  ImageUrl(ImageUrl&&) noexcept = default;

  ImageUrl& operator=(ImageUrl&&) noexcept = default;

  ImageUrl(const ImageUrl&) = delete;

  ImageUrl& operator=(const ImageUrl&) = delete;
};

// References an image URL in the content of a message.
struct ImageUrlContent : Content {

  // The type of the content part.
  ImageUrlContent(const std::string& type) : Content(type) {}

  ImageUrlContent(ImageUrlContent&&) noexcept = default;

  ImageUrlContent& operator=(ImageUrlContent&&) noexcept = default;

  ImageUrlContent(const ImageUrlContent&) = delete;

  ImageUrlContent& operator=(const ImageUrlContent&) = delete;

  ImageUrl image_url;

  static cpp::result<ImageUrlContent, std::string> FromJson(
      Json::Value&& json) {
    if (json.empty()) {
      return cpp::fail("Json string is empty");
    }

    try {
      ImageUrlContent content{"image_url"};
      ImageUrl image_url;
      image_url.url = std::move(json["image_url"]["url"].asString());
      image_url.detail = std::move(json["image_url"]["detail"].asString());
      content.image_url = std::move(image_url);
      return content;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJson failed: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["type"] = type;
      json["image_url"]["url"] = image_url.url;
      json["image_url"]["detail"] = image_url.detail;
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
}  // namespace ThreadMessage
