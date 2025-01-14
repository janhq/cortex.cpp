#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX
#undef min
#undef max
#endif
#include "template_renderer.h"
#include <regex>
#include <stdexcept>
#include "utils/logging_utils.h"
#include "utils/string_utils.h"
namespace extensions {

TemplateRenderer::TemplateRenderer() {
  // Configure Inja environment
  env_.set_trim_blocks(true);
  env_.set_lstrip_blocks(true);

  // Add tojson function for all value types
  env_.add_callback("tojson", 1, [](inja::Arguments& args) {
    if (args.empty()) {
      return nlohmann::json(nullptr);
    }
    const auto& value = *args[0];

    if (value.is_string()) {
      return nlohmann::json(std::string("\"") +
                            string_utils::EscapeJson(value.get<std::string>()) +
                            "\"");
    }
    return value;
  });
}

std::string TemplateRenderer::Render(const std::string& tmpl,
                                     const Json::Value& data) {
  try {
    // Convert Json::Value to nlohmann::json
    auto json_data = ConvertJsonValue(data);

    // Create the input data structure expected by the template
    nlohmann::json template_data;
    template_data["input_request"] = json_data;

    // Debug output
    LOG_DEBUG << "Template: " << tmpl;
    LOG_DEBUG << "Data: " << template_data.dump(2);

    // Render template
    std::string result = env_.render(tmpl, template_data);

    // Clean up any potential double quotes in JSON strings
    // result = std::regex_replace(result, std::regex("\\\"\\\""), "\"");

    LOG_DEBUG << "Result: " << result;

    return result;
  } catch (const std::exception& e) {
    LOG_ERROR << "Template rendering failed: " << e.what();
    LOG_ERROR << "Data: " << data.toStyledString();
    LOG_ERROR << "Template: " << tmpl;
    throw std::runtime_error(std::string("Template rendering failed: ") +
                             e.what());
  }
}

nlohmann::json TemplateRenderer::ConvertJsonValue(const Json::Value& input) {
  if (input.isNull()) {
    return nullptr;
  } else if (input.isBool()) {
    return input.asBool();
  } else if (input.isInt()) {
    return input.asInt();
  } else if (input.isUInt()) {
    return input.asUInt();
  } else if (input.isDouble()) {
    return input.asDouble();
  } else if (input.isString()) {
    return input.asString();
  } else if (input.isArray()) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& element : input) {
      arr.push_back(ConvertJsonValue(element));
    }
    return arr;
  } else if (input.isObject()) {
    nlohmann::json obj = nlohmann::json::object();
    for (const auto& key : input.getMemberNames()) {
      obj[key] = ConvertJsonValue(input[key]);
    }
    return obj;
  }
  return nullptr;
}

Json::Value TemplateRenderer::ConvertNlohmannJson(const nlohmann::json& input) {
  if (input.is_null()) {
    return Json::Value();
  } else if (input.is_boolean()) {
    return Json::Value(input.get<bool>());
  } else if (input.is_number_integer()) {
    return Json::Value(input.get<int>());
  } else if (input.is_number_unsigned()) {
    return Json::Value(input.get<unsigned int>());
  } else if (input.is_number_float()) {
    return Json::Value(input.get<double>());
  } else if (input.is_string()) {
    return Json::Value(input.get<std::string>());
  } else if (input.is_array()) {
    Json::Value arr(Json::arrayValue);
    for (const auto& element : input) {
      arr.append(ConvertNlohmannJson(element));
    }
    return arr;
  } else if (input.is_object()) {
    Json::Value obj(Json::objectValue);
    for (auto it = input.begin(); it != input.end(); ++it) {
      obj[it.key()] = ConvertNlohmannJson(it.value());
    }
    return obj;
  }
  return Json::Value();
}

std::string TemplateRenderer::RenderFile(const std::string& template_path,
                                         const Json::Value& data) {
  try {
    // Convert Json::Value to nlohmann::json
    auto json_data = ConvertJsonValue(data);

    // Load and render template
    return env_.render_file(template_path, json_data);
  } catch (const std::exception& e) {
    throw std::runtime_error(std::string("Template file rendering failed: ") +
                             e.what());
  }
}
}  // namespace extensions