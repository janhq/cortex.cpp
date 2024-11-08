#include "remote_engine.h"
#include <sstream>

// Static callback function for CURL
static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

RemoteEngine::RemoteEngine() {
    curl_global_init(CURL_GLOBAL_ALL);
}

RemoteEngine::~RemoteEngine() {
    curl_global_cleanup();
}

CurlResponse RemoteEngine::makeRequest(const std::string& url, 
                                     const std::string& api_key,
                                     const std::string& body,
                                     const std::string& method) {
    CURL* curl = curl_easy_init();
    CurlResponse response;
    
    if (!curl) {
        response.error = true;
        response.error_message = "Failed to initialize CURL";
        return response;
    }

    // Set up headers
    struct curl_slist* headers = nullptr;
    if (!api_key.empty()) {
        std::string auth_header = renderTemplate(config_.api_key_template, {{"api_key", api_key}});
        headers = curl_slist_append(headers, auth_header.c_str());
    }
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }

    std::string response_string;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        response.error = true;
        response.error_message = curl_easy_strerror(res);
    } else {
        response.body = response_string;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

std::string RemoteEngine::renderTemplate(const std::string& templ, 
                                       const std::unordered_map<std::string, std::string>& values) {
    std::string result = templ;
    for (const auto& [key, value] : values) {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = result.find(placeholder);
        if (pos != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
        }
    }
    return result;
}

Json::Value RemoteEngine::transformRequest(const Json::Value& input, const std::string& type) {
    if (!config_.transform_req.isMember(type)) {
        return input;
    }

    Json::Value output = input;
    const Json::Value& transforms = config_.transform_req[type];
    
    for (const auto& transform : transforms) {
        if (transform.isString()) {
            // Handle template-based transformation
            if (transform.asString().find("template") != std::string::npos) {
                // Implement template rendering logic here
                continue;
            }
        } else if (transform.isObject()) {
            // Handle key mapping transformations
            for (const auto& key : transform.getMemberNames()) {
                if (input.isMember(key)) {
                    output[transform[key].asString()] = input[key];
                    output.removeMember(key);
                }
            }
        }
    }
    return output;
}

void RemoteEngine::GetModels(std::shared_ptr<Json::Value> json_body,
                           std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
    if (!json_body->isMember("url") || !json_body->isMember("api_key")) {
        Json::Value error;
        error["error"] = "Missing required fields: url or api_key";
        callback(Json::Value(), std::move(error));
        return;
    }

    const std::string& url = (*json_body)["url"].asString();
    const std::string& api_key = (*json_body)["api_key"].asString();

    auto response = makeRequest(url, api_key, "", "GET");
    
    if (response.error) {
        Json::Value error;
        error["error"] = response.error_message;
        callback(Json::Value(), std::move(error));
        return;
    }

    Json::Value response_json;
    Json::Reader reader;
    if (!reader.parse(response.body, response_json)) {
        Json::Value error;
        error["error"] = "Failed to parse response";
        callback(Json::Value(), std::move(error));
        return;
    }

    callback(std::move(response_json), Json::Value());
}

void RemoteEngine::HandleChatCompletion(std::shared_ptr<Json::Value> json_body,
                                      std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
    if (!json_body->isMember("url") || !json_body->isMember("api_key") || 
        !json_body->isMember("request_body")) {
        Json::Value error;
        error["error"] = "Missing required fields: url, api_key, or request_body";
        callback(Json::Value(), std::move(error));
        return;
    }

    const std::string& url = (*json_body)["url"].asString();
    const std::string& api_key = (*json_body)["api_key"].asString();
    
    Json::Value transformed_request = transformRequest((*json_body)["request_body"], "chat_completion");
    
    Json::FastWriter writer;
    std::string request_body = writer.write(transformed_request);

    auto response = makeRequest(url, api_key, request_body);
    
    if (response.error) {
        Json::Value error;
        error["error"] = response.error_message;
        callback(Json::Value(), std::move(error));
        return;
    }

    Json::Value response_json;
    Json::Reader reader;
    if (!reader.parse(response.body, response_json)) {
        Json::Value error;
        error["error"] = "Failed to parse response";
        callback(Json::Value(), std::move(error));
        return;
    }

    callback(std::move(response_json), Json::Value());
}

bool RemoteEngine::LoadConfig(const std::string& yaml_path) {
    try {
        YAML::Node config = YAML::LoadFile(yaml_path);
        
        if (config["api_key_template"]) {
            config_.api_key_template = config["api_key_template"].as<std::string>();
        }

        if (config["TransformReq"]) {
            Json::Reader reader;
            reader.parse(config["TransformReq"].as<std::string>(), config_.transform_req);
        }

        if (config["TransformResp"]) {
            Json::Reader reader;
            reader.parse(config["TransformResp"].as<std::string>(), config_.transform_resp);
        }

        return true;
    } catch (const YAML::Exception& e) {
        LOG_ERROR << "Failed to load config: " << e.what();
        return false;
    }
}

// Implement other virtual functions with minimal functionality
void RemoteEngine::HandleEmbedding(std::shared_ptr<Json::Value>, 
                                 std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
    callback(Json::Value(), Json::Value());
}

void RemoteEngine::LoadModel(std::shared_ptr<Json::Value>,
                           std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
    callback(Json::Value(), Json::Value());
}

void RemoteEngine::UnloadModel(std::shared_ptr<Json::Value>,
                             std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
    callback(Json::Value(), Json::Value());
}

void RemoteEngine::GetModelStatus(std::shared_ptr<Json::Value>,
                                std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
    callback(Json::Value(), Json::Value());
}

bool RemoteEngine::IsSupported(const std::string&) {
    return true;
}

bool RemoteEngine::SetFileLogger(int, const std::string&) {
    return true;
}

void RemoteEngine::SetLogLevel(trantor::Logger::LogLevel) {
}