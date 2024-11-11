#include "TemplateRenderer.h"
#include <stdexcept>
#include <regex>
#include <json/writer.h>
#include <json/reader.h>

TemplateRenderer::TemplateRenderer() {
    // Configure Inja environment
    env_.set_trim_blocks(true);
    env_.set_lstrip_blocks(true);

    // Add tojson function for all value types
    env_.add_callback("tojson", 1, [](inja::Arguments& args) {
        if (args.empty()) {
            return inja::json(nullptr);
        }
        const auto& value = *args[0];
        
        if (value.is_string()) {
            return inja::json(std::string("\"") + value.get<std::string>() + "\"");
        }
        return value;
    });
}

std::string TemplateRenderer::jsonToString(const Json::Value& value) {
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, value);
}

bool TemplateRenderer::validateJson(const std::string& jsonStr) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream iss(jsonStr);
    return Json::parseFromStream(builder, iss, &root, &errors);
}

std::string TemplateRenderer::render(const std::string& tmpl, const Json::Value& data) {
    try {
        // Create the input data structure expected by the template
        Json::Value template_data;
        template_data["input_request"] = data;
        
        // Convert to string for logging
        std::string dataStr = jsonToString(template_data);
        
        // Debug output
        LOG_DEBUG << "Template: " << tmpl;
        LOG_DEBUG << "Data: " << dataStr;
        
        // Convert to inja's json format
        auto inja_data = inja::json::parse(dataStr);
        
        // Render template
        std::string result = env_.render(tmpl, inja_data);
        
        // Clean up any potential double quotes in JSON strings
        result = std::regex_replace(result, std::regex("\\\"\\\""), "\"");
        
        LOG_DEBUG << "Result: " << result;
        
        // Validate JSON
        if (!validateJson(result)) {
            throw std::runtime_error("Invalid JSON in rendered template");
        }
        
        return result;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "Template rendering failed: " << e.what();
        LOG_ERROR << "Template: " << tmpl;
        throw std::runtime_error(std::string("Template rendering failed: ") + e.what());
    }
}

std::string TemplateRenderer::renderFile(const std::string& template_path, const Json::Value& data) {
    try {
        // Convert JsonCpp Value to string
        std::string dataStr = jsonToString(data);
        
        // Parse as inja json
        auto inja_data = inja::json::parse(dataStr);
        
        // Load and render template
        return env_.render_file(template_path, inja_data);
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Template file rendering failed: ") + e.what());
    }
}