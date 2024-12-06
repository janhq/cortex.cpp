#pragma once

#include <optional>
#include <string>

namespace OpenAi {
struct AssistantTool {
  std::string type;

  AssistantTool(const std::string& type) : type{type} {}

  virtual ~AssistantTool() = default;
};

struct AssistantCodeInterpreterTool : public AssistantTool {
  AssistantCodeInterpreterTool() : AssistantTool{"code_interpreter"} {}

  ~AssistantCodeInterpreterTool() = default;
};

struct AssistantFileSearchTool : public AssistantTool {
  AssistantFileSearchTool() : AssistantTool("file_search") {}

  ~AssistantFileSearchTool() = default;

  /**
   * The ranking options for the file search. If not specified,
   * the file search tool will use the auto ranker and a score_threshold of 0.
   *
   * See the file search tool documentation for more information.
   */
  struct RankingOption {
    /**
     * The ranker to use for the file search. If not specified will use the auto ranker.
     */
    std::string ranker;

    /**
     * The score threshold for the file search. All values must be a
     * floating point number between 0 and 1.
     */
    float score_threshold;
  };

  /**
   * Overrides for the file search tool.
   */
  struct FileSearch {
    /**
     * The maximum number of results the file search tool should output.
     * The default is 20 for gpt-4* models and 5 for gpt-3.5-turbo.
     * This number should be between 1 and 50 inclusive.
     *
     * Note that the file search tool may output fewer than max_num_results results.
     * See the file search tool documentation for more information.
     */
    int max_num_result;
  };
};

struct AssistantFunctionTool : public AssistantTool {
  AssistantFunctionTool() : AssistantTool("function") {}

  ~AssistantFunctionTool() = default;

  struct Function {
    /**
     * A description of what the function does, used by the model to choose
     * when and how to call the function.
     */
    std::string description;

    /**
     * The name of the function to be called. Must be a-z, A-Z, 0-9, or contain
     * underscores and dashes, with a maximum length of 64.
     */
    std::string name;

    // TODO: namh handle parameters

    /**
     * Whether to enable strict schema adherence when generating the function call.
     * If set to true, the model will follow the exact schema defined in the parameters
     * field. Only a subset of JSON Schema is supported when strict is true.
     *
     * Learn more about Structured Outputs in the function calling guide.
     */
    std::optional<bool> strict;
  };
};
}  // namespace OpenAi
