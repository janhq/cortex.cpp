#pragma once

#include <json/reader.h>
#include "common/json_serializable.h"

namespace OpenAi {
// The tools to add this file to.
struct Tool {
  std::string type;

  Tool(const std::string& type) : type{type} {}

  virtual ~Tool() = default;
};

// The type of tool being defined: code_interpreter
struct MessageCodeInterpreter : Tool {
  MessageCodeInterpreter() : Tool{"code_interpreter"} {}

  ~MessageCodeInterpreter() = default;
};

// The type of tool being defined: file_search
struct MessageFileSearch : Tool {
  MessageFileSearch() : Tool{"file_search"} {}

  ~MessageFileSearch() = default;
};

// A list of files attached to the message, and the tools they were added to.
struct Attachment : JsonSerializable {

  // The ID of the file to attach to the message.
  std::string file_id;

  std::vector<Tool> tools;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["file_id"] = file_id;
      Json::Value tools_json_arr{Json::arrayValue};
      for (auto& tool : tools) {
        Json::Value tool_json;
        tool_json["type"] = tool.type;
        tools_json_arr.append(tool_json);
      }
      json["tools"] = tools_json_arr;
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
};  // namespace OpenAi
