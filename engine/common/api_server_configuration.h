#pragma once

#include <json/value.h>
#include <functional>
#include <unordered_map>
#include <vector>

// current only support basic auth
enum class ProxyAuthMethod {
  Basic,
  Digest,
  DigestIe,
  Bearer,
  Negotiate,
  Ntlm,
  NtlmWb,
  Any,
  AnySafe,
  AuthOnly,
  AwsSigV4
};

struct ApiConfigurationMetadata {
  std::string name;
  std::string desc;
  std::string group;
  std::string accept_value;
  std::string default_value;

  bool allow_empty = false;
};

static const std::unordered_map<std::string, ApiConfigurationMetadata>
    CONFIGURATIONS = {
        {"cors",
         ApiConfigurationMetadata{
             .name = "cors",
             .desc = "Cross-Origin Resource Sharing configuration.",
             .group = "CORS",
             .accept_value = "[on|off]",
             .default_value = "on"}},
        {"allowed_origins",
         ApiConfigurationMetadata{
             .name = "allowed_origins",
             .desc = "Allowed origins for CORS. Comma separated. E.g. "
                     "http://localhost,https://cortex.so",
             .group = "CORS",
             .accept_value = "comma separated",
             .default_value = "*",
             .allow_empty = true}},
        {"proxy_url", ApiConfigurationMetadata{.name = "proxy_url",
                                               .desc = "Proxy URL",
                                               .group = "Proxy",
                                               .accept_value = "string",
                                               .default_value = ""}},
        {"proxy_username", ApiConfigurationMetadata{.name = "proxy_username",
                                                    .desc = "Proxy Username",
                                                    .group = "Proxy",
                                                    .accept_value = "string",
                                                    .default_value = ""}},
        {"proxy_password", ApiConfigurationMetadata{.name = "proxy_password",
                                                    .desc = "Proxy Password",
                                                    .group = "Proxy",
                                                    .accept_value = "string",
                                                    .default_value = ""}},
        {"verify_proxy_ssl",
         ApiConfigurationMetadata{.name = "verify_proxy_ssl",
                                  .desc = "Verify SSL for proxy",
                                  .group = "Proxy",
                                  .accept_value = "[on|off]",
                                  .default_value = "on"}},
        {"verify_proxy_host_ssl",
         ApiConfigurationMetadata{.name = "verify_proxy_host_ssl",
                                  .desc = "Verify SSL for proxy",
                                  .group = "Proxy",
                                  .accept_value = "[on|off]",
                                  .default_value = "on"}},
        {"no_proxy", ApiConfigurationMetadata{.name = "no_proxy",
                                              .desc = "No proxy for hosts",
                                              .group = "Proxy",
                                              .accept_value = "string",
                                              .default_value = ""}},
        {"verify_peer_ssl", ApiConfigurationMetadata{.name = "verify_peer_ssl",
                                                     .desc = "Verify peer SSL",
                                                     .group = "Proxy",
                                                     .accept_value = "[on|off]",
                                                     .default_value = "on"}},
        {"verify_host_ssl", ApiConfigurationMetadata{.name = "verify_host_ssl",
                                                     .desc = "Verify host SSL",
                                                     .group = "Proxy",
                                                     .accept_value = "[on|off]",
                                                     .default_value = "on"}},
        {"huggingface_token",
         ApiConfigurationMetadata{.name = "huggingface_token",
                                  .desc = "HuggingFace token to pull models",
                                  .group = "Token",
                                  .accept_value = "string",
                                  .default_value = "",
                                  .allow_empty = true}},
        {"github_token", ApiConfigurationMetadata{.name = "github_token",
                                                  .desc = "Github token",
                                                  .group = "Token",
                                                  .accept_value = "string",
                                                  .default_value = "",
                                                  .allow_empty = true}},
};

class ApiServerConfiguration {
 public:
  ApiServerConfiguration(
      bool cors = true, std::vector<std::string> allowed_origins = {},
      bool verify_proxy_ssl = true, bool verify_proxy_host_ssl = true,
      const std::string& proxy_url = "", const std::string& proxy_username = "",
      const std::string& proxy_password = "", const std::string& no_proxy = "",
      bool verify_peer_ssl = true, bool verify_host_ssl = true,
      const std::string& hf_token = "", const std::string& gh_token = "")
      : cors{cors},
        allowed_origins{allowed_origins},
        verify_proxy_ssl{verify_proxy_ssl},
        verify_proxy_host_ssl{verify_proxy_host_ssl},
        proxy_url{proxy_url},
        proxy_username{proxy_username},
        proxy_password{proxy_password},
        no_proxy{no_proxy},
        verify_peer_ssl{verify_peer_ssl},
        verify_host_ssl{verify_host_ssl},
        hf_token{hf_token},
        gh_token{gh_token} {}

  // cors
  bool cors{true};
  std::vector<std::string> allowed_origins;

  // proxy
  bool verify_proxy_ssl{true};
  bool verify_proxy_host_ssl{true};
  ProxyAuthMethod proxy_auth_method{ProxyAuthMethod::Basic};
  std::string proxy_url{""};
  std::string proxy_username{""};
  std::string proxy_password{""};
  std::string no_proxy{""};

  bool verify_peer_ssl{true};
  bool verify_host_ssl{true};

  // token
  std::string hf_token{""};
  std::string gh_token{""};

  Json::Value ToJson() const {
    Json::Value root;
    root["cors"] = cors;
    root["allowed_origins"] = Json::Value(Json::arrayValue);
    for (const auto& origin : allowed_origins) {
      root["allowed_origins"].append(origin);
    }
    root["verify_proxy_ssl"] = verify_proxy_ssl;
    root["verify_proxy_host_ssl"] = verify_proxy_host_ssl;
    root["proxy_url"] = proxy_url;
    root["proxy_username"] = proxy_username;
    root["proxy_password"] = proxy_password;
    root["no_proxy"] = no_proxy;
    root["verify_peer_ssl"] = verify_peer_ssl;
    root["verify_host_ssl"] = verify_host_ssl;
    root["huggingface_token"] = hf_token;
    root["github_token"] = gh_token;

    return root;
  }

  void UpdateFromJson(const Json::Value& json,
                      std::vector<std::string>* updated_fields = nullptr,
                      std::vector<std::string>* invalid_fields = nullptr,
                      std::vector<std::string>* unknown_fields = nullptr) {
    const std::unordered_map<std::string,
                             std::function<bool(const Json::Value&)>>
        field_updater{
            {"verify_peer_ssl",
             [this](const Json::Value& value) -> bool {
               if (!value.isBool()) {
                 return false;
               }
               verify_peer_ssl = value.asBool();
               return true;
             }},

            {"verify_host_ssl",
             [this](const Json::Value& value) -> bool {
               if (!value.isBool()) {
                 return false;
               }
               verify_host_ssl = value.asBool();
               return true;
             }},

            {"verify_proxy_host_ssl",
             [this](const Json::Value& value) -> bool {
               if (!value.isBool()) {
                 return false;
               }
               verify_proxy_host_ssl = value.asBool();
               return true;
             }},

            {"verify_proxy_ssl",
             [this](const Json::Value& value) -> bool {
               if (!value.isBool()) {
                 return false;
               }
               verify_proxy_ssl = value.asBool();
               return true;
             }},

            {"no_proxy",
             [this](const Json::Value& value) -> bool {
               if (!value.isString()) {
                 return false;
               }
               no_proxy = value.asString();
               return true;
             }},

            {"proxy_url",
             [this](const Json::Value& value) -> bool {
               if (!value.isString()) {
                 return false;
               }
               proxy_url = value.asString();
               return true;
             }},

            {"proxy_username",
             [this](const Json::Value& value) -> bool {
               if (!value.isString()) {
                 return false;
               }
               proxy_username = value.asString();
               return true;
             }},

            {"proxy_password",
             [this](const Json::Value& value) -> bool {
               if (!value.isString()) {
                 return false;
               }
               proxy_password = value.asString();
               return true;
             }},

            {"huggingface_token",
             [this](const Json::Value& value) -> bool {
               if (!value.isString()) {
                 return false;
               }
               hf_token = value.asString();
               return true;
             }},

            {"github_token",
             [this](const Json::Value& value) -> bool {
               if (!value.isString()) {
                 return false;
               }
               gh_token = value.asString();
               return true;
             }},

            {"cors",
             [this](const Json::Value& value) -> bool {
               if (!value.isBool()) {
                 return false;
               }
               cors = value.asBool();
               return true;
             }},

            {"allowed_origins", [this](const Json::Value& value) -> bool {
               if (!value.isArray()) {
                 return false;
               }
               for (const auto& origin : value) {
                 if (!origin.isString()) {
                   return false;
                 }
               }

               this->allowed_origins.clear();
               for (const auto& origin : value) {
                 this->allowed_origins.push_back(origin.asString());
               }
               return true;
             }}};

    for (const auto& key : json.getMemberNames()) {
      auto updater = field_updater.find(key);
      if (updater != field_updater.end()) {
        if (updater->second(json[key])) {
          if (updated_fields != nullptr) {
            updated_fields->push_back(key);
          }
        } else {
          if (invalid_fields != nullptr) {
            invalid_fields->push_back(key);
          }
        }
      } else {
        if (unknown_fields != nullptr) {
          unknown_fields->push_back(key);
        }
      }
    }
  };
};
