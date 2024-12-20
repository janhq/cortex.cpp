#pragma once

#include <string>
#include <vector>
#include "common/json_serializable.h"

namespace OpenAi {

struct ThreadToolResources : JsonSerializable {
  ~ThreadToolResources() = default;

  virtual cpp::result<Json::Value, std::string> ToJson() override = 0;
};

struct ThreadCodeInterpreter : ThreadToolResources {
  std::vector<std::string> file_ids;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      Json::Value file_ids_json{Json::arrayValue};
      for (auto& file_id : file_ids) {
        file_ids_json.append(file_id);
      }
      json["file_ids"] = file_ids_json;
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};

struct ThreadFileSearch : ThreadToolResources {
  std::vector<std::string> vector_store_ids;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      Json::Value vector_store_ids_json{Json::arrayValue};
      for (auto& vector_store_id : vector_store_ids) {
        vector_store_ids_json.append(vector_store_id);
      }
      json["vector_store_ids"] = vector_store_ids_json;
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
}  // namespace OpenAi
