#pragma once

#include <json/value.h>
#include <string>

#include "extensions/remote-engine/template_renderer.h"
#include "utils/chat-template.hpp"
#include "utils/result.hpp"

namespace jinja {
inline cpp::result<std::string, std::string> RenderTemplate(
    std::string& tmpl, const Json::Value& data, const std::string& bos_token,
    const std::string& eos_token) {
  try {
    auto nlohmann_json =
        remote_engine::TemplateRenderer().ConvertJsonValue(data);

    minja::chat_template chat_tmpl(tmpl, bos_token, eos_token);
    auto add_generation_prompt = false;
    if (!nlohmann_json["add_generation_prompt"].is_null()) {
      add_generation_prompt = nlohmann_json["add_generation_prompt"];
    }
    return chat_tmpl.apply(
        /*messages*/ nlohmann_json["messages"],
        /*tools*/ {},  //nlohmann_json["tools"],
        /*add_generation_prompt*/ add_generation_prompt);
  } catch (const std::exception& e) {
    return cpp::fail("Failed to render template: " + std::string(e.what()));
  }
}
}  // namespace jinja
