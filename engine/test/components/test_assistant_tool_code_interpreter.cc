#include <gtest/gtest.h>
#include <json/json.h>
#include "common/assistant_code_interpreter_tool.h"

namespace OpenAi {
namespace {

class AssistantCodeInterpreterToolTest : public ::testing::Test {};

TEST_F(AssistantCodeInterpreterToolTest, BasicConstruction) {
  AssistantCodeInterpreterTool tool;
  EXPECT_EQ(tool.type, "code_interpreter");
}

TEST_F(AssistantCodeInterpreterToolTest, MoveConstructor) {
  AssistantCodeInterpreterTool original;
  AssistantCodeInterpreterTool moved(std::move(original));
  EXPECT_EQ(moved.type, "code_interpreter");
}

TEST_F(AssistantCodeInterpreterToolTest, MoveAssignment) {
  AssistantCodeInterpreterTool original;
  AssistantCodeInterpreterTool target;
  target = std::move(original);
  EXPECT_EQ(target.type, "code_interpreter");
}

TEST_F(AssistantCodeInterpreterToolTest, FromJson) {
  Json::Value json;  // Empty JSON is fine for this tool
  auto result = AssistantCodeInterpreterTool::FromJson();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value().type, "code_interpreter");
}

TEST_F(AssistantCodeInterpreterToolTest, ToJson) {
  AssistantCodeInterpreterTool tool;
  auto result = tool.ToJson();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value()["type"].asString(), "code_interpreter");

  // Verify no extra fields
  Json::Value::Members members = result.value().getMemberNames();
  EXPECT_EQ(members.size(), 1);  // Only "type" field should be present
  EXPECT_EQ(members[0], "type");
}
}  // namespace
}  // namespace OpenAi
