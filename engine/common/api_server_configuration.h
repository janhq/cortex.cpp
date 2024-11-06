#pragma once

#include <json/value.h>
#include <unordered_map>
#include <vector>

class ApiServerConfiguration {
 public:
  ApiServerConfiguration(bool cors = true,
                         std::vector<std::string> allowed_origins = {})
      : cors{cors}, allowed_origins{allowed_origins} {}

  bool cors{true};
  std::vector<std::string> allowed_origins;

  Json::Value ToJson() const {
    Json::Value root;
    root["cors"] = cors;
    root["allowed_origins"] = Json::Value(Json::arrayValue);
    for (const auto& origin : allowed_origins) {
      root["allowed_origins"].append(origin);
    }
    return root;
  }

  void UpdateFromJson(const Json::Value& json,
                      std::vector<std::string>* updated_fields = nullptr,
                      std::vector<std::string>* invalid_fields = nullptr,
                      std::vector<std::string>* unknown_fields = nullptr) {
    const std::unordered_map<std::string,
                             std::function<bool(const Json::Value&)>>
        field_updater{
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
