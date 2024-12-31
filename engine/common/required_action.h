#pragma once

#include <string>
#include <vector>
#include "common/json_serializable.h"

namespace OpenAi {
struct ToolCall {
  /**
   * The ID of the tool call. This ID must be referenced when you submit
   * the tool outputs in using the Submit tool outputs to run endpoint.
   */
  std::string id;

  /**
   * The type of tool call the output is required for. For now, this is
   * always function.
   */
  std::string type{"function"};

  // function TODO: NamH implement this
};

struct SubmitToolOutputs {
  std::vector<ToolCall> tool_calls;
};

struct RequiredAction : JsonSerializable {
  std::string type{"submit_tool_outputs"};

  SubmitToolOutputs submit_tool_outputs;

  cpp::result<Json::Value, std::string> ToJson() const {
    Json::Value root;
    // TODO: NamH implement this
    return root;
  }
};
}  // namespace OpenAi
