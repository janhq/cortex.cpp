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

class ApiServerConfiguration {
 public:
  ApiServerConfiguration(bool cors = true,
                         std::vector<std::string> allowed_origins = {},
                         bool verify_proxy_ssl = true,
                         const std::string& proxy_url = "",
                         const std::string& proxy_username = "",
                         const std::string& proxy_password = "")
      : cors{cors},
        allowed_origins{allowed_origins},
        proxy_url{proxy_url},
        proxy_username{proxy_username},
        proxy_password{proxy_password} {}

  // cors
  bool cors{true};
  std::vector<std::string> allowed_origins;

  // proxy
  bool verify_proxy_ssl{true};
  ProxyAuthMethod proxy_auth_method{ProxyAuthMethod::Basic};
  std::string proxy_url{""};
  std::string proxy_username{""};
  std::string proxy_password{""};
  // TODO: namh should we support proxy headers?
  // TODO: namh should we allow configurable timeout? proxy add overhead to request

  // just placeholder here
  bool verify_peer_ssl{true};
  bool verify_host_ssl{true};

  Json::Value ToJson() const {
    Json::Value root;
    root["cors"] = cors;
    root["allowed_origins"] = Json::Value(Json::arrayValue);
    for (const auto& origin : allowed_origins) {
      root["allowed_origins"].append(origin);
    }
    root["verify_proxy_ssl"] = verify_proxy_ssl;
    // TODO: namh add proxy auth method
    root["proxy_url"] = proxy_url;
    root["proxy_username"] = proxy_username;
    root["proxy_password"] = proxy_password;

    return root;
  }

  void UpdateFromJson(const Json::Value& json,
                      std::vector<std::string>* updated_fields = nullptr,
                      std::vector<std::string>* invalid_fields = nullptr,
                      std::vector<std::string>* unknown_fields = nullptr) {
    const std::unordered_map<std::string,
                             std::function<bool(const Json::Value&)>>
        field_updater{
            {"verify_proxy_ssl",
             [this](const Json::Value& value) -> bool {
               if (!value.isBool()) {
                 return false;
               }
               verify_proxy_ssl = value.asBool();
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
