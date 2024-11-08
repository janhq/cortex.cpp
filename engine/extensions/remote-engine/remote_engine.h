#pragma once

#include "cortex-common/EngineI.h"
#include <json/json.h>
#include <curl/curl.h>
#include <yaml-cpp/yaml.h>
#include <string>
#include <unordered_map>

// Helper for CURL response
struct CurlResponse {
    std::string body;
    bool error{false};
    std::string error_message;
};

class RemoteEngine : public EngineI {
private:
    // Store config from YAML
    struct Config {
        std::string api_key_template;
        Json::Value transform_req;
        Json::Value transform_resp;
    };
    
    Config config_;
    
    // Helper functions
    CurlResponse makeRequest(const std::string& url, 
                           const std::string& api_key,
                           const std::string& body,
                           const std::string& method = "POST");

    std::string renderTemplate(const std::string& templ, 
                             const std::unordered_map<std::string, std::string>& values);

    Json::Value transformRequest(const Json::Value& input, const std::string& type);

public:
    RemoteEngine();
    ~RemoteEngine();

    // Main interface implementations
    void GetModels(std::shared_ptr<Json::Value> json_body,
                  std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

    void HandleChatCompletion(std::shared_ptr<Json::Value> json_body,
                            std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

    // Config loading
    bool LoadConfig(const std::string& yaml_path);

    // Other required virtual functions
    void HandleEmbedding(std::shared_ptr<Json::Value> json_body,
                        std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
    void LoadModel(std::shared_ptr<Json::Value> json_body,
                  std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
    void UnloadModel(std::shared_ptr<Json::Value> json_body,
                    std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
    void GetModelStatus(std::shared_ptr<Json::Value> json_body,
                       std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;
    bool IsSupported(const std::string& feature) override;
    bool SetFileLogger(int max_log_lines, const std::string& log_path) override;
    void SetLogLevel(trantor::Logger::LogLevel logLevel) override;
};