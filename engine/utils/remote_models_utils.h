#pragma once

#include <yaml-cpp/yaml.h>
#include <json/json.h>
#include <string>

namespace remote_models_utils {

inline Json::Value yamlToJson(const YAML::Node& node) {
    Json::Value result;

    switch (node.Type()) {
        case YAML::NodeType::Null:
            return Json::Value();
        case YAML::NodeType::Scalar: {
            // For scalar types, we'll first try to parse as string
            std::string str_val = node.as<std::string>();
            
            // Try to parse as boolean
            if (str_val == "true" || str_val == "True" || str_val == "TRUE")
                return Json::Value(true);
            if (str_val == "false" || str_val == "False" || str_val == "FALSE")
                return Json::Value(false);
            
            // Try to parse as number
            try {
                // Check if it's an integer
                size_t pos;
                long long int_val = std::stoll(str_val, &pos);
                if (pos == str_val.length()) {
                    return Json::Value(static_cast<Json::Int64>(int_val));
                }

                // Check if it's a float
                double float_val = std::stod(str_val, &pos);
                if (pos == str_val.length()) {
                    return Json::Value(float_val);
                }
            } catch (...) {
                // If parsing as number fails, use as string
            }
            
            // Default to string if no other type matches
            return Json::Value(str_val);
        }
        case YAML::NodeType::Sequence: {
            result = Json::Value(Json::arrayValue);
            for (const auto& elem : node) {
                result.append(yamlToJson(elem));
            }
            return result;
        }
        case YAML::NodeType::Map: {
            result = Json::Value(Json::objectValue);
            for (const auto& it : node) {
                std::string key = it.first.as<std::string>();
                result[key] = yamlToJson(it.second);
            }
            return result;
        }
        default:
            return Json::Value();
    }
}

inline YAML::Node jsonToYaml(const Json::Value& json) {
    YAML::Node result;
    
    switch (json.type()) {
        case Json::nullValue:
            result = YAML::Node(YAML::NodeType::Null);
            break;
        case Json::intValue:
            result = json.asInt64();
            break;
        case Json::uintValue:
            result = json.asUInt64();
            break;
        case Json::realValue:
            result = json.asDouble();
            break;
        case Json::stringValue:
            result = json.asString();
            break;
        case Json::booleanValue:
            result = json.asBool();
            break;
        case Json::arrayValue:
            result = YAML::Node(YAML::NodeType::Sequence);
            for (const auto& elem : json)
                result.push_back(jsonToYaml(elem));
            break;
        case Json::objectValue:
            result = YAML::Node(YAML::NodeType::Map);
            for (const auto& key : json.getMemberNames())
                result[key] = jsonToYaml(json[key]);
            break;
    }
    return result;
}

} // namespace utils