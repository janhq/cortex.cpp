#pragma once

#include "common/message_content.h"

namespace OpenAi {

struct ImageUrl : public JsonSerializable {
  /**
   * The external URL of the image, must be a supported image types:
   * jpeg, jpg, png, gif, webp.
   */
  std::string url;

  /**
   * Specifies the detail level of the image. low uses fewer tokens, you
   * can opt in to high resolution using high. Default value is auto
   */
  std::string detail;

  ImageUrl(const std::string& url, const std::string& detail = "auto")
      : url{url}, detail{detail} {}

  ImageUrl(ImageUrl&&) noexcept = default;

  ImageUrl& operator=(ImageUrl&&) noexcept = default;

  ImageUrl(const ImageUrl&) = delete;

  ImageUrl& operator=(const ImageUrl&) = delete;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value root;
      root["url"] = url;
      root["detail"] = detail;
      return root;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};

// References an image URL in the content of a message.
struct ImageUrlContent : Content {

  // The type of the content part.
  explicit ImageUrlContent(const std::string& type, ImageUrl&& image_url)
      : Content(type), image_url{std::move(image_url)} {}

  ImageUrlContent(ImageUrlContent&&) noexcept = default;

  ImageUrlContent& operator=(ImageUrlContent&&) noexcept = default;

  ImageUrlContent(const ImageUrlContent&) = delete;

  ImageUrlContent& operator=(const ImageUrlContent&) = delete;

  ImageUrl image_url;

  ~ImageUrlContent() override = default;

  static cpp::result<ImageUrlContent, std::string> FromJson(
      Json::Value&& json) {
    if (json.empty()) {
      return cpp::fail("Json string is empty");
    }

    try {
      auto image_url = ImageUrl(json["image_url"]["url"].asString(),
                                json["image_url"]["detail"].asString());
      ImageUrlContent content{"image_url", std::move(image_url)};
      return content;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJson failed: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["type"] = type;
      json["image_url"] = image_url.ToJson().value();
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
}  // namespace OpenAi
