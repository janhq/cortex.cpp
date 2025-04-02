#include <memory>
#include "gtest/gtest.h"
#include "json/json.h"
#include "utils/function_calling/common.h"

class FunctionCallingUtilsTest : public ::testing::Test {
 protected:
  std::shared_ptr<Json::Value> createTestRequest() {
    auto request = std::make_shared<Json::Value>();
    (*request)["tools"] = Json::Value(Json::arrayValue);
    return request;
  }
};

TEST_F(FunctionCallingUtilsTest, ReplaceCustomFunctions) {
  std::string original = "Test <CUSTOM_FUNCTIONS> placeholder";
  std::string replacement = "Custom function";
  std::string result =
      function_calling_utils::ReplaceCustomFunctions(original, replacement);
  EXPECT_EQ(result, "Test Custom function placeholder");
}

TEST_F(FunctionCallingUtilsTest, HasTools) {
  auto request = createTestRequest();
  EXPECT_FALSE(function_calling_utils::HasTools(request));

  (*request)["tools"].append(Json::Value());
  EXPECT_TRUE(function_calling_utils::HasTools(request));

  (*request)["tools"] = "random";
  EXPECT_FALSE(function_calling_utils::HasTools(request));

  (*request)["tools"] = Json::Value(Json::nullValue);
  EXPECT_FALSE(function_calling_utils::HasTools(request));
}

TEST_F(FunctionCallingUtilsTest, ProcessTools) {
  auto request = createTestRequest();
  Json::Value tool;
  tool["type"] = "function";
  tool["function"]["name"] = "test_function";
  tool["function"]["description"] = "Test description";
  (*request)["tools"].append(tool);

  std::string result = function_calling_utils::ProcessTools(request);
  EXPECT_TRUE(
      result.find("Use the function 'test_function' to: Test description") !=
      std::string::npos);
}

TEST_F(FunctionCallingUtilsTest, ParseMultipleFunctionStrings) {
  std::string input =
      "<function=func1>{\"arg\":\"value1\"}</"
      "function><function=func2>{\"arg\":\"value2\"}</function>";
  Json::Value result =
      function_calling_utils::ParseMultipleFunctionStrings(input);

  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0]["function"]["name"].asString(), "func1");
  EXPECT_EQ(result[0]["function"]["arguments"].asString(),
            "{\"arg\":\"value1\"}");
  EXPECT_EQ(result[1]["function"]["name"].asString(), "func2");
  EXPECT_EQ(result[1]["function"]["arguments"].asString(),
            "{\"arg\":\"value2\"}");
}

TEST_F(FunctionCallingUtilsTest, ConvertJsonToFunctionStrings) {
  Json::Value jsonArray(Json::arrayValue);
  Json::Value function1, function2;
  function1["function"]["name"] = "func1";
  function1["function"]["arguments"] = "{\"arg\":\"value1\"}";
  function2["function"]["name"] = "func2";
  function2["function"]["arguments"] = "{\"arg\":\"value2\"}";
  jsonArray.append(function1);
  jsonArray.append(function2);

  std::string result =
      function_calling_utils::ConvertJsonToFunctionStrings(jsonArray);
  EXPECT_EQ(result,
            "<function=func1>{\"arg\":\"value1\"}</"
            "function><function=func2>{\"arg\":\"value2\"}</function>");
}

TEST_F(FunctionCallingUtilsTest, CreateCustomFunctionsString) {
  auto request = createTestRequest();
  Json::Value tool;
  tool["type"] = "function";
  tool["function"]["name"] = "test_function";
  tool["function"]["description"] = "Test description";
  (*request)["tools"].append(tool);

  std::string result =
      function_calling_utils::CreateCustomFunctionsString(request);
  EXPECT_TRUE(result.find("```") != std::string::npos);
  EXPECT_TRUE(
      result.find("Use the function 'test_function' to: Test description") !=
      std::string::npos);
}

TEST_F(FunctionCallingUtilsTest, IsValidToolChoiceFormat) {
  Json::Value validTool;
  validTool["type"] = "function";
  validTool["function"]["name"] = "test_function";
  EXPECT_TRUE(function_calling_utils::IsValidToolChoiceFormat(validTool));

  Json::Value invalidTool;
  EXPECT_FALSE(function_calling_utils::IsValidToolChoiceFormat(invalidTool));
}

TEST_F(FunctionCallingUtilsTest, UpdateMessages) {
  auto request = createTestRequest();
  std::string system_prompt = "Original prompt";
  (*request)["messages"] = Json::Value(Json::arrayValue);

  function_calling_utils::UpdateMessages(system_prompt, request);

  ASSERT_TRUE((*request)["messages"].isArray());
  EXPECT_EQ((*request)["messages"][0]["role"].asString(), "system");
  EXPECT_EQ((*request)["messages"][0]["content"].asString(), system_prompt);
}

TEST_F(FunctionCallingUtilsTest, PreprocessRequest) {
  auto request = createTestRequest();
  Json::Value tool;
  tool["type"] = "function";
  tool["function"]["name"] = "test_function";
  tool["function"]["description"] = "Test description";
  (*request)["tools"].append(tool);

  function_calling_utils::PreprocessRequest(request);

  ASSERT_TRUE((*request)["messages"].isArray());
  EXPECT_TRUE((*request)["messages"][0]["content"].asString().find(
                  "Test description") != std::string::npos);
}

TEST_F(FunctionCallingUtilsTest, PostProcessResponse) {
  Json::Value response;
  response["choices"] = Json::Value(Json::arrayValue);
  Json::Value choice;
  choice["message"]["content"] =
      "<function=test_function>{\"arg\":\"value\"}</function>";
  response["choices"].append(choice);

  function_calling_utils::PostProcessResponse(response);

  EXPECT_EQ(response["choices"][0]["message"]["content"].asString(), "");
  EXPECT_TRUE(response["choices"][0]["message"]["tool_calls"].isArray());
  EXPECT_EQ(
      response["choices"][0]["message"]["tool_calls"][0]["function"]["name"]
          .asString(),
      "test_function");
  EXPECT_EQ(response["choices"][0]["message"]["tool_calls"][0]["function"]
                    ["arguments"]
                        .asString(),
            "{\"arg\":\"value\"}");
}