#include <gtest/gtest.h>
#include "common/assistant.h"

namespace OpenAi {
namespace {

class AssistantTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up base assistant with minimal required fields
    base_assistant.id = "asst_123";
    base_assistant.object = "assistant";
    base_assistant.created_at = 1702000000;
    base_assistant.model = "gpt-4";
  }

  Assistant base_assistant;
};

TEST_F(AssistantTest, MinimalAssistantToJson) {
  auto result = base_assistant.ToJson();
  ASSERT_TRUE(result.has_value());
  
  Json::Value json = result.value();
  EXPECT_EQ(json["id"].asString(), "asst_123");
  EXPECT_EQ(json["object"].asString(), "assistant");
  EXPECT_EQ(json["created_at"].asUInt64(), 1702000000);
  EXPECT_EQ(json["model"].asString(), "gpt-4");
}

TEST_F(AssistantTest, FullAssistantToJson) {
  base_assistant.name = "Test Assistant";
  base_assistant.description = "Test Description";
  base_assistant.instructions = "Test Instructions";
  base_assistant.temperature = 0.7f;
  base_assistant.top_p = 0.9f;
  
  // Add a code interpreter tool
  auto code_tool = std::make_unique<AssistantCodeInterpreterTool>();
  base_assistant.tools.push_back(std::move(code_tool));
  
  // Add metadata
  base_assistant.metadata["key1"] = std::string("value1");
  base_assistant.metadata["key2"] = true;
  base_assistant.metadata["key3"] = static_cast<uint64_t>(42ULL);
  
  auto result = base_assistant.ToJson();
  ASSERT_TRUE(result.has_value());
  
  Json::Value json = result.value();
  EXPECT_EQ(json["name"].asString(), "Test Assistant");
  EXPECT_EQ(json["description"].asString(), "Test Description");
  EXPECT_EQ(json["instructions"].asString(), "Test Instructions");
  EXPECT_FLOAT_EQ(json["temperature"].asFloat(), 0.7f);
  EXPECT_FLOAT_EQ(json["top_p"].asFloat(), 0.9f);
  
  EXPECT_TRUE(json["tools"].isArray());
  EXPECT_EQ(json["tools"].size(), 1);
  EXPECT_EQ(json["tools"][0]["type"].asString(), "code_interpreter");
  
  EXPECT_TRUE(json["metadata"].isObject());
  EXPECT_EQ(json["metadata"]["key1"].asString(), "value1");
  EXPECT_EQ(json["metadata"]["key2"].asBool(), true);
  EXPECT_EQ(json["metadata"]["key3"].asUInt64(), 42ULL);
}

TEST_F(AssistantTest, FromJsonMinimal) {
  Json::Value input;
  input["id"] = "asst_123";
  input["object"] = "assistant";
  input["created_at"] = 1702000000;
  input["model"] = "gpt-4";
  
  auto result = Assistant::FromJson(std::move(input));
  ASSERT_TRUE(result.has_value());
  
  const auto& assistant = result.value();
  EXPECT_EQ(assistant.id, "asst_123");
  EXPECT_EQ(assistant.object, "assistant");
  EXPECT_EQ(assistant.created_at, 1702000000);
  EXPECT_EQ(assistant.model, "gpt-4");
}

TEST_F(AssistantTest, FromJsonComplete) {
  Json::Value input;
  input["id"] = "asst_123";
  input["object"] = "assistant";
  input["created_at"] = 1702000000;
  input["model"] = "gpt-4";
  input["name"] = "Test Assistant";
  input["description"] = "Test Description";
  input["instructions"] = "Test Instructions";
  input["temperature"] = 0.7;
  input["top_p"] = 0.9;
  
  // Add tools
  Json::Value tools(Json::arrayValue);
  Json::Value code_tool;
  code_tool["type"] = "code_interpreter";
  tools.append(code_tool);
  
  Json::Value function_tool;
  function_tool["type"] = "function";
  function_tool["function"] = Json::Value(Json::objectValue);
  function_tool["function"]["name"] = "test_function";
  function_tool["function"]["description"] = "Test function";
  function_tool["function"]["parameters"] = Json::Value(Json::objectValue);
  tools.append(function_tool);
  input["tools"] = tools;
  
  // Add metadata
  Json::Value metadata(Json::objectValue);
  metadata["key1"] = "value1";
  metadata["key2"] = true;
  metadata["key3"] = 42;
  input["metadata"] = metadata;
  
  auto result = Assistant::FromJson(std::move(input));
  ASSERT_TRUE(result.has_value());
  
  const auto& assistant = result.value();
  EXPECT_EQ(assistant.name.value(), "Test Assistant");
  EXPECT_EQ(assistant.description.value(), "Test Description");
  EXPECT_EQ(assistant.instructions.value(), "Test Instructions");
  EXPECT_FLOAT_EQ(assistant.temperature.value(), 0.7f);
  EXPECT_FLOAT_EQ(assistant.top_p.value(), 0.9f);
  
  EXPECT_EQ(assistant.tools.size(), 2);
  EXPECT_TRUE(dynamic_cast<AssistantCodeInterpreterTool*>(assistant.tools[0].get()) != nullptr);
  EXPECT_TRUE(dynamic_cast<AssistantFunctionTool*>(assistant.tools[1].get()) != nullptr);
  
  EXPECT_EQ(assistant.metadata.size(), 3);
  EXPECT_EQ(std::get<std::string>(assistant.metadata.at("key1")), "value1");
  EXPECT_EQ(std::get<bool>(assistant.metadata.at("key2")), true);
  EXPECT_EQ(std::get<uint64_t>(assistant.metadata.at("key3")), 42ULL);
}

TEST_F(AssistantTest, FromJsonInvalidInput) {
  // Missing required field 'id'
  {
    Json::Value input;
    input["object"] = "assistant";
    input["created_at"] = 1702000000;
    input["model"] = "gpt-4";
    
    auto result = Assistant::FromJson(std::move(input));
    EXPECT_FALSE(result.has_value());
  }
  
  // Invalid object type
  {
    Json::Value input;
    input["id"] = "asst_123";
    input["object"] = "invalid";
    input["created_at"] = 1702000000;
    input["model"] = "gpt-4";
    
    auto result = Assistant::FromJson(std::move(input));
    EXPECT_FALSE(result.has_value());
  }
  
  // Invalid created_at type
  {
    Json::Value input;
    input["id"] = "asst_123";
    input["object"] = "assistant";
    input["created_at"] = "invalid";
    input["model"] = "gpt-4";
    
    auto result = Assistant::FromJson(std::move(input));
    EXPECT_FALSE(result.has_value());
  }
}

TEST_F(AssistantTest, MoveConstructorAndAssignment) {
  base_assistant.name = "Test Assistant";
  base_assistant.tools.push_back(std::make_unique<AssistantCodeInterpreterTool>());
  
  // Test move constructor
  Assistant moved_assistant(std::move(base_assistant));
  EXPECT_EQ(moved_assistant.id, "asst_123");
  EXPECT_EQ(moved_assistant.name.value(), "Test Assistant");
  EXPECT_EQ(moved_assistant.tools.size(), 1);
  
  // Test move assignment
  Assistant another_assistant;
  another_assistant = std::move(moved_assistant);
  EXPECT_EQ(another_assistant.id, "asst_123");
  EXPECT_EQ(another_assistant.name.value(), "Test Assistant");
  EXPECT_EQ(another_assistant.tools.size(), 1);
}

}  // namespace
}  // namespace OpenAi
