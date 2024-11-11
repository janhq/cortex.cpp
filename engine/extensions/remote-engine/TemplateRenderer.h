#pragma once

#include <string>
#include <memory>
#include <inja/inja.hpp>
#include "json/json.h"
#include "trantor/utils/Logger.h"

class TemplateRenderer {
public:
    TemplateRenderer();
    ~TemplateRenderer() = default;

    // Render template with data
    std::string render(const std::string& tmpl, const Json::Value& data);

    // Load template from file and render
    std::string renderFile(const std::string& template_path, const Json::Value& data);

private:
    // Helper function to convert JsonCpp Value to string representation
    static std::string jsonToString(const Json::Value& value);
    
    // Helper function to validate JSON string
    static bool validateJson(const std::string& jsonStr);

    inja::Environment env_;
};