#pragma once

#include <string>
#include <vector>
#include "common/json_serializable.h"

namespace OpenAi {

struct ToolResources : JsonSerializable {
  ToolResources() = default;

  ToolResources(const ToolResources&) = delete;

  ToolResources& operator=(const ToolResources&) = delete;

  ToolResources(ToolResources&&) noexcept = default;

  ToolResources& operator=(ToolResources&&) noexcept = default;

  virtual ~ToolResources() = default;

  virtual cpp::result<Json::Value, std::string> ToJson() override = 0;
};

struct CodeInterpreter : ToolResources {
  CodeInterpreter() = default;

  ~CodeInterpreter() override = default;

  CodeInterpreter(const CodeInterpreter&) = delete;

  CodeInterpreter& operator=(const CodeInterpreter&) = delete;

  CodeInterpreter(CodeInterpreter&& other) noexcept
      : ToolResources(std::move(other)), file_ids(std::move(other.file_ids)) {}

  CodeInterpreter& operator=(CodeInterpreter&& other) noexcept {
    if (this != &other) {
      ToolResources::operator=(std::move(other));
      file_ids = std::move(other.file_ids);
    }
    return *this;
  }

  std::vector<std::string> file_ids;

  static cpp::result<CodeInterpreter, std::string> FromJson(
      const Json::Value& json) {
    CodeInterpreter code_interpreter;
    if (json.isMember("file_ids")) {
      for (const auto& file_id : json["file_ids"]) {
        code_interpreter.file_ids.push_back(file_id.asString());
      }
    }
    return code_interpreter;
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value json;
    Json::Value file_ids_json{Json::arrayValue};
    for (auto& file_id : file_ids) {
      file_ids_json.append(file_id);
    }
    json["file_ids"] = file_ids_json;
    return json;
  }
};

struct FileSearch : ToolResources {
  FileSearch() = default;

  ~FileSearch() override = default;

  FileSearch(const FileSearch&) = delete;

  FileSearch& operator=(const FileSearch&) = delete;

  FileSearch(FileSearch&& other) noexcept
      : ToolResources(std::move(other)),
        vector_store_ids{std::move(other.vector_store_ids)} {}

  FileSearch& operator=(FileSearch&& other) noexcept {
    if (this != &other) {
      ToolResources::operator=(std::move(other));

      vector_store_ids = std::move(other.vector_store_ids);
    }
    return *this;
  }

  std::vector<std::string> vector_store_ids;

  static cpp::result<FileSearch, std::string> FromJson(
      const Json::Value& json) {
    FileSearch file_search;
    if (json.isMember("vector_store_ids")) {
      for (const auto& vector_store_id : json["vector_store_ids"]) {
        file_search.vector_store_ids.push_back(vector_store_id.asString());
      }
    }
    return file_search;
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value json;
    Json::Value vector_store_ids_json{Json::arrayValue};
    for (auto& vector_store_id : vector_store_ids) {
      vector_store_ids_json.append(vector_store_id);
    }
    json["vector_store_ids"] = vector_store_ids_json;
    return json;
  }
};
}  // namespace OpenAi
