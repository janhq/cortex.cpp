#pragma once

#include <string>
#include <vector>

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

struct RequiredAction {
  std::string type{"submit_tool_outputs"};

  SubmitToolOutputs submit_tool_outputs;
};
}  // namespace OpenAi
