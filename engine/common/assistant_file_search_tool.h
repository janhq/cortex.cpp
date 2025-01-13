#pragma once

#include "common/assistant_tool.h"
#include "common/json_serializable.h"

namespace OpenAi {
struct FileSearchRankingOption : public JsonSerializable {
  /**
  * The ranker to use for the file search. If not specified will use the auto ranker.
     */
  std::string ranker;

  /**
     * The score threshold for the file search. All values must be a
     * floating point number between 0 and 1.
     */
  float score_threshold;

  FileSearchRankingOption(float score_threshold,
                          const std::string& ranker = "auto")
      : ranker{ranker}, score_threshold{score_threshold} {}

  FileSearchRankingOption(const FileSearchRankingOption&) = delete;

  FileSearchRankingOption& operator=(const FileSearchRankingOption&) = delete;

  FileSearchRankingOption(FileSearchRankingOption&&) = default;

  FileSearchRankingOption& operator=(FileSearchRankingOption&&) = default;

  ~FileSearchRankingOption() = default;

  static cpp::result<FileSearchRankingOption, std::string> FromJson(
      const Json::Value& json) {
    if (!json.isMember("score_threshold")) {
      return cpp::fail("score_threshold must be provided");
    }

    FileSearchRankingOption option{
        json["score_threshold"].asFloat(),
        std::move(json.get("ranker", "auto").asString())};
    return option;
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value json;
    json["ranker"] = ranker;
    json["score_threshold"] = score_threshold;
    return json;
  }
};

/**
   * Overrides for the file search tool.
   */
struct AssistantFileSearch : public JsonSerializable {
  /**
     * The maximum number of results the file search tool should output.
     * The default is 20 for gpt-4* models and 5 for gpt-3.5-turbo.
     * This number should be between 1 and 50 inclusive.
     *
     * Note that the file search tool may output fewer than max_num_results results.
     * See the file search tool documentation for more information.
     */
  int max_num_results;

  /**
     * The ranking options for the file search. If not specified,
     * the file search tool will use the auto ranker and a score_threshold of 0.
     *
     * See the file search tool documentation for more information.
     */
  FileSearchRankingOption ranking_options;

  AssistantFileSearch(int max_num_results,
                      FileSearchRankingOption&& ranking_options)
      : max_num_results{max_num_results},
        ranking_options{std::move(ranking_options)} {}

  AssistantFileSearch(const AssistantFileSearch&) = delete;

  AssistantFileSearch& operator=(const AssistantFileSearch&) = delete;

  AssistantFileSearch(AssistantFileSearch&&) = default;

  AssistantFileSearch& operator=(AssistantFileSearch&&) = default;

  ~AssistantFileSearch() = default;

  static cpp::result<AssistantFileSearch, std::string> FromJson(
      const Json::Value& json) {
    try {
      AssistantFileSearch search{
          json["max_num_results"].asInt(),
          FileSearchRankingOption::FromJson(json["ranking_options"]).value()};
      return search;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJson failed: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    Json::Value root;
    root["max_num_results"] = max_num_results;
    root["ranking_options"] = ranking_options.ToJson().value();
    return root;
  }
};

struct AssistantFileSearchTool : public AssistantTool {
  AssistantFileSearch file_search;

  AssistantFileSearchTool(AssistantFileSearch& file_search)
      : AssistantTool("file_search"), file_search{std::move(file_search)} {}

  AssistantFileSearchTool(const AssistantFileSearchTool&) = delete;

  AssistantFileSearchTool& operator=(const AssistantFileSearchTool&) = delete;

  AssistantFileSearchTool(AssistantFileSearchTool&&) = default;

  AssistantFileSearchTool& operator=(AssistantFileSearchTool&&) = default;

  ~AssistantFileSearchTool() = default;

  static cpp::result<AssistantFileSearchTool, std::string> FromJson(
      const Json::Value& json) {
    try {
      AssistantFileSearch search{json["file_search"]["max_num_results"].asInt(),
                                 FileSearchRankingOption::FromJson(
                                     json["file_search"]["ranking_options"])
                                     .value()};
      AssistantFileSearchTool tool{search};
      return tool;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJson failed: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value root;
      root["type"] = type;
      root["file_search"] = file_search.ToJson().value();
      return root;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
};  // namespace OpenAi
