#pragma once

#include <json/value.h>
#include <string>

#include "extensions/template_renderer.h"
#include "utils/chat-template.hpp"
#include "utils/result.hpp"

namespace jinja {
inline cpp::result<std::string, std::string> RenderTemplate(
    std::string& tmpl, const Json::Value& data, const std::string& bos_token,
    const std::string& eos_token, bool add_bos_token, bool add_eos_token,
    bool add_generation_prompt = true) {
  try {
    auto converted_json =
        extensions::TemplateRenderer().ConvertJsonValue(data);

    minja::chat_template chat_tmpl(tmpl, add_bos_token ? bos_token : "",
                                   add_eos_token ? eos_token : "");
    return chat_tmpl.apply(converted_json["messages"], {},
                           add_generation_prompt);
  } catch (const std::exception& e) {
    return cpp::fail("Failed to render template: " + std::string(e.what()));
  }
}
}  // namespace jinja
