#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include <inja/inja.hpp>
#include "json/json.h"
#include "trantor/utils/Logger.h"
class TemplateRenderer {
public:
    TemplateRenderer();
    ~TemplateRenderer() = default;

    // Convert Json::Value to nlohmann::json
    static nlohmann::json convertJsonValue(const Json::Value& input);
    
    // Convert nlohmann::json to Json::Value
    static Json::Value convertNlohmannJson(const nlohmann::json& input);

    // Render template with data
    std::string render(const std::string& tmpl, const Json::Value& data);

    // Load template from file and render
    std::string renderFile(const std::string& template_path, const Json::Value& data);

private:
    inja::Environment env_;
};