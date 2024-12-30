#include <gtest/gtest.h>
#include <json/json.h>
#include "common/assistant_file_search_tool.h"

namespace OpenAi {
namespace {

class AssistantFileSearchToolTest : public ::testing::Test {};

TEST_F(AssistantFileSearchToolTest, FileSearchRankingOptionBasicConstruction) {
  const float threshold = 0.75f;
  const std::string ranker = "test_ranker";
  FileSearchRankingOption option{threshold, ranker};

  EXPECT_EQ(option.score_threshold, threshold);
  EXPECT_EQ(option.ranker, ranker);
}

TEST_F(AssistantFileSearchToolTest, FileSearchRankingOptionDefaultRanker) {
  const float threshold = 0.5f;
  FileSearchRankingOption option{threshold};

  EXPECT_EQ(option.score_threshold, threshold);
  EXPECT_EQ(option.ranker, "auto");
}

TEST_F(AssistantFileSearchToolTest, FileSearchRankingOptionFromValidJson) {
  Json::Value json;
  json["score_threshold"] = 0.8f;
  json["ranker"] = "custom_ranker";

  auto result = FileSearchRankingOption::FromJson(json);
  ASSERT_TRUE(result.has_value());

  EXPECT_EQ(result.value().score_threshold, 0.8f);
  EXPECT_EQ(result.value().ranker, "custom_ranker");
}

TEST_F(AssistantFileSearchToolTest, FileSearchRankingOptionFromInvalidJson) {
  Json::Value json;
  auto result = FileSearchRankingOption::FromJson(json);
  EXPECT_FALSE(result.has_value());
}

TEST_F(AssistantFileSearchToolTest, FileSearchRankingOptionToJson) {
  FileSearchRankingOption option{0.9f, "special_ranker"};
  auto json_result = option.ToJson();

  ASSERT_TRUE(json_result.has_value());
  Json::Value json = json_result.value();

  EXPECT_EQ(json["score_threshold"].asFloat(), 0.9f);
  EXPECT_EQ(json["ranker"].asString(), "special_ranker");
}

TEST_F(AssistantFileSearchToolTest, AssistantFileSearchBasicConstruction) {
  FileSearchRankingOption ranking_option{0.7f, "test_ranker"};
  AssistantFileSearch search{10, std::move(ranking_option)};

  EXPECT_EQ(search.max_num_results, 10);
  EXPECT_EQ(search.ranking_options.score_threshold, 0.7f);
  EXPECT_EQ(search.ranking_options.ranker, "test_ranker");
}

TEST_F(AssistantFileSearchToolTest, AssistantFileSearchFromValidJson) {
  Json::Value json;
  json["max_num_results"] = 15;

  Json::Value ranking_json;
  ranking_json["score_threshold"] = 0.85f;
  ranking_json["ranker"] = "custom_ranker";
  json["ranking_options"] = ranking_json;

  auto result = AssistantFileSearch::FromJson(json);
  ASSERT_TRUE(result.has_value());

  EXPECT_EQ(result.value().max_num_results, 15);
  EXPECT_EQ(result.value().ranking_options.score_threshold, 0.85f);
  EXPECT_EQ(result.value().ranking_options.ranker, "custom_ranker");
}

TEST_F(AssistantFileSearchToolTest, AssistantFileSearchFromInvalidJson) {
  Json::Value json;
  // Missing required fields
  auto result = AssistantFileSearch::FromJson(json);
  EXPECT_FALSE(result.has_value());
}

TEST_F(AssistantFileSearchToolTest, AssistantFileSearchToJson) {
  FileSearchRankingOption ranking_option{0.95f, "advanced_ranker"};
  AssistantFileSearch search{20, std::move(ranking_option)};

  auto json_result = search.ToJson();
  ASSERT_TRUE(json_result.has_value());

  Json::Value json = json_result.value();
  EXPECT_EQ(json["max_num_results"].asInt(), 20);
  EXPECT_EQ(json["ranking_options"]["score_threshold"].asFloat(), 0.95f);
  EXPECT_EQ(json["ranking_options"]["ranker"].asString(), "advanced_ranker");
}

TEST_F(AssistantFileSearchToolTest, AssistantFileSearchToolConstruction) {
  FileSearchRankingOption ranking_option{0.8f, "tool_ranker"};
  AssistantFileSearch search{25, std::move(ranking_option)};
  AssistantFileSearchTool tool{search};

  EXPECT_EQ(tool.type, "file_search");
  EXPECT_EQ(tool.file_search.max_num_results, 25);
  EXPECT_EQ(tool.file_search.ranking_options.score_threshold, 0.8f);
  EXPECT_EQ(tool.file_search.ranking_options.ranker, "tool_ranker");
}

TEST_F(AssistantFileSearchToolTest, AssistantFileSearchToolFromValidJson) {
  Json::Value json;
  json["type"] = "file_search";

  Json::Value file_search;
  file_search["max_num_results"] = 30;

  Json::Value ranking_options;
  ranking_options["score_threshold"] = 0.75f;
  ranking_options["ranker"] = "json_ranker";
  file_search["ranking_options"] = ranking_options;

  json["file_search"] = file_search;

  auto result = AssistantFileSearchTool::FromJson(json);
  ASSERT_TRUE(result.has_value());

  EXPECT_EQ(result.value().type, "file_search");
  EXPECT_EQ(result.value().file_search.max_num_results, 30);
  EXPECT_EQ(result.value().file_search.ranking_options.score_threshold, 0.75f);
  EXPECT_EQ(result.value().file_search.ranking_options.ranker, "json_ranker");
}

TEST_F(AssistantFileSearchToolTest, AssistantFileSearchToolFromInvalidJson) {
  Json::Value json;
  // Missing required fields
  auto result = AssistantFileSearchTool::FromJson(json);
  EXPECT_FALSE(result.has_value());
}

TEST_F(AssistantFileSearchToolTest, AssistantFileSearchToolToJson) {
  FileSearchRankingOption ranking_option{0.65f, "final_ranker"};
  AssistantFileSearch search{35, std::move(ranking_option)};
  AssistantFileSearchTool tool{search};

  auto json_result = tool.ToJson();
  ASSERT_TRUE(json_result.has_value());

  Json::Value json = json_result.value();
  EXPECT_EQ(json["type"].asString(), "file_search");
  EXPECT_EQ(json["file_search"]["max_num_results"].asInt(), 35);
  EXPECT_EQ(json["file_search"]["ranking_options"]["score_threshold"].asFloat(),
            0.65f);
  EXPECT_EQ(json["file_search"]["ranking_options"]["ranker"].asString(),
            "final_ranker");
}

TEST_F(AssistantFileSearchToolTest, MoveConstructorsAndAssignments) {
  // Test FileSearchRankingOption move operations
  FileSearchRankingOption original_option{0.8f, "original_ranker"};
  FileSearchRankingOption moved_option{std::move(original_option)};
  EXPECT_EQ(moved_option.score_threshold, 0.8f);
  EXPECT_EQ(moved_option.ranker, "original_ranker");

  FileSearchRankingOption assign_target{0.5f};
  assign_target = std::move(moved_option);
  EXPECT_EQ(assign_target.score_threshold, 0.8f);
  EXPECT_EQ(assign_target.ranker, "original_ranker");

  // Test AssistantFileSearch move operations
  FileSearchRankingOption search_option{0.9f, "search_ranker"};
  AssistantFileSearch original_search{40, std::move(search_option)};
  AssistantFileSearch moved_search{std::move(original_search)};
  EXPECT_EQ(moved_search.max_num_results, 40);
  EXPECT_EQ(moved_search.ranking_options.score_threshold, 0.9f);

  // Test AssistantFileSearchTool move operations
  FileSearchRankingOption tool_option{0.7f, "tool_ranker"};
  AssistantFileSearch tool_search{45, std::move(tool_option)};
  AssistantFileSearchTool original_tool{tool_search};
  AssistantFileSearchTool moved_tool{std::move(original_tool)};
  EXPECT_EQ(moved_tool.type, "file_search");
  EXPECT_EQ(moved_tool.file_search.max_num_results, 45);
}

TEST_F(AssistantFileSearchToolTest, EdgeCases) {
  // Test boundary values for score_threshold
  FileSearchRankingOption min_threshold{0.0f};
  EXPECT_EQ(min_threshold.score_threshold, 0.0f);

  FileSearchRankingOption max_threshold{1.0f};
  EXPECT_EQ(max_threshold.score_threshold, 1.0f);

  // Test boundary values for max_num_results
  FileSearchRankingOption ranking_option{0.5f};
  AssistantFileSearch min_results{1, std::move(ranking_option)};
  EXPECT_EQ(min_results.max_num_results, 1);

  FileSearchRankingOption ranking_option2{0.5f};
  AssistantFileSearch max_results{50, std::move(ranking_option2)};
  EXPECT_EQ(max_results.max_num_results, 50);
}

}  // namespace
}  // namespace OpenAi
