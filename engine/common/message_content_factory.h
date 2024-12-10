#pragma once

#include <json/value.h>
#include "common/message_content_image_file.h"
#include "common/message_content_image_url.h"
#include "common/message_content_refusal.h"
#include "common/message_content_text.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"

namespace OpenAi {
inline cpp::result<std::unique_ptr<Content>, std::string> ParseContent(
    Json::Value&& json) {
  if (json.empty()) {
    return cpp::fail("Json string is empty");
  }

  try {
    auto type = json["type"].asString();

    if (type == "image_file") {
      auto result = ImageFileContent::FromJson(std::move(json));
      if (result.has_error()) {
        return cpp::fail(result.error());
      }
      return std::make_unique<ImageFileContent>(std::move(result.value()));
    } else if (type == "image_url") {
      auto result = ImageUrlContent::FromJson(std::move(json));
      if (result.has_error()) {
        return cpp::fail(result.error());
      }
      return std::make_unique<ImageUrlContent>(std::move(result.value()));
    } else if (type == "text") {
      auto result = TextContent::FromJson(std::move(json));
      if (result.has_error()) {
        return cpp::fail(result.error());
      }
      return std::make_unique<TextContent>(std::move(result.value()));
    } else if (type == "refusal") {
      auto result = Refusal::FromJson(std::move(json));
      if (result.has_error()) {
        return cpp::fail(result.error());
      }
      return std::make_unique<Refusal>(std::move(result.value()));
    } else {
      return cpp::fail("Unknown content type: " + type);
    }

    return cpp::fail("Unknown content type: " + type);
  } catch (const std::exception& e) {
    return cpp::fail(std::string("ParseContent failed: ") + e.what());
  }
}

inline cpp::result<std::vector<std::unique_ptr<Content>>, std::string>
ParseContents(Json::Value&& json) {
  if (json.empty()) {
    return cpp::fail("Json string is empty");
  }
  if (!json.isArray()) {
    return cpp::fail("Json is not an array");
  }

  std::vector<std::unique_ptr<Content>> contents;
  Json::Value mutable_json = std::move(json);

  for (auto& content_json : mutable_json) {
    auto content = ParseContent(std::move(content_json));
    if (content.has_error()) {
      CTL_WRN(content.error());
      continue;
    }
    contents.push_back(std::move(content.value()));
  }
  return contents;
}
}  // namespace OpenAi
