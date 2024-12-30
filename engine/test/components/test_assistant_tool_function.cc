#include <gtest/gtest.h>
#include "common/assistant_function_tool.h"
#include <json/json.h>

namespace OpenAi {
namespace {

class AssistantFunctionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common test setup
        basic_description = "Test function description";
        basic_name = "test_function";
        basic_params = Json::Value(Json::objectValue);
        basic_params["type"] = "object";
        basic_params["properties"] = Json::Value(Json::objectValue);
    }

    std::string basic_description;
    std::string basic_name;
    Json::Value basic_params;
};

TEST_F(AssistantFunctionTest, BasicConstructionWithoutStrict) {
    AssistantFunction function(basic_description, basic_name, basic_params, std::nullopt);
    
    EXPECT_EQ(function.description, basic_description);
    EXPECT_EQ(function.name, basic_name);
    EXPECT_EQ(function.parameters, basic_params);
    EXPECT_FALSE(function.strict.has_value());
}

TEST_F(AssistantFunctionTest, BasicConstructionWithStrict) {
    AssistantFunction function(basic_description, basic_name, basic_params, true);
    
    EXPECT_EQ(function.description, basic_description);
    EXPECT_EQ(function.name, basic_name);
    EXPECT_EQ(function.parameters, basic_params);
    ASSERT_TRUE(function.strict.has_value());
    EXPECT_TRUE(*function.strict);
}

TEST_F(AssistantFunctionTest, MoveConstructor) {
    AssistantFunction original(basic_description, basic_name, basic_params, true);
    
    AssistantFunction moved(std::move(original));
    
    EXPECT_EQ(moved.description, basic_description);
    EXPECT_EQ(moved.name, basic_name);
    EXPECT_EQ(moved.parameters, basic_params);
    ASSERT_TRUE(moved.strict.has_value());
    EXPECT_TRUE(*moved.strict);
}

TEST_F(AssistantFunctionTest, MoveAssignment) {
    AssistantFunction original(basic_description, basic_name, basic_params, true);
    
    AssistantFunction target("other", "other_name", Json::Value(Json::objectValue), false);
    target = std::move(original);
    
    EXPECT_EQ(target.description, basic_description);
    EXPECT_EQ(target.name, basic_name);
    EXPECT_EQ(target.parameters, basic_params);
    ASSERT_TRUE(target.strict.has_value());
    EXPECT_TRUE(*target.strict);
}

TEST_F(AssistantFunctionTest, FromValidJson) {
    Json::Value json;
    json["description"] = basic_description;
    json["name"] = basic_name;
    json["strict"] = true;
    json["parameters"] = basic_params;
    
    auto result = AssistantFunction::FromJson(json);
    ASSERT_TRUE(result.has_value());
    
    const auto& function = result.value();
    EXPECT_EQ(function.description, basic_description);
    EXPECT_EQ(function.name, basic_name);
    EXPECT_EQ(function.parameters, basic_params);
    ASSERT_TRUE(function.strict.has_value());
    EXPECT_TRUE(*function.strict);
}

TEST_F(AssistantFunctionTest, FromJsonValidationEmptyJson) {
    Json::Value json;
    auto result = AssistantFunction::FromJson(json);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.error(), "Function json can't be empty");
}

TEST_F(AssistantFunctionTest, FromJsonValidationEmptyName) {
    Json::Value json;
    json["description"] = basic_description;
    json["parameters"] = basic_params;
    
    auto result = AssistantFunction::FromJson(json);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.error(), "Function name can't be empty");

    // Test with empty name value
    json["name"] = "";
    result = AssistantFunction::FromJson(json);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.error(), "Function name can't be empty");
}

TEST_F(AssistantFunctionTest, FromJsonValidationMissingDescription) {
    Json::Value json;
    json["name"] = basic_name;
    json["parameters"] = basic_params;
    
    auto result = AssistantFunction::FromJson(json);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.error(), "Function description is mandatory");
}

TEST_F(AssistantFunctionTest, FromJsonValidationMissingParameters) {
    Json::Value json;
    json["name"] = basic_name;
    json["description"] = basic_description;
    
    auto result = AssistantFunction::FromJson(json);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.error(), "Function parameters are mandatory");
}

TEST_F(AssistantFunctionTest, ToJsonWithStrict) {
    AssistantFunction function(basic_description, basic_name, basic_params, true);
    
    auto result = function.ToJson();
    ASSERT_TRUE(result.has_value());
    
    const auto& json = result.value();
    EXPECT_EQ(json["description"].asString(), basic_description);
    EXPECT_EQ(json["name"].asString(), basic_name);
    EXPECT_EQ(json["parameters"], basic_params);
    EXPECT_TRUE(json["strict"].asBool());
}

TEST_F(AssistantFunctionTest, ToJsonWithoutStrict) {
    AssistantFunction function(basic_description, basic_name, basic_params, std::nullopt);
    
    auto result = function.ToJson();
    ASSERT_TRUE(result.has_value());
    
    const auto& json = result.value();
    EXPECT_EQ(json["description"].asString(), basic_description);
    EXPECT_EQ(json["name"].asString(), basic_name);
    EXPECT_EQ(json["parameters"], basic_params);
    EXPECT_FALSE(json.isMember("strict"));
}

// AssistantFunctionTool Tests
class AssistantFunctionToolTest : public ::testing::Test {
protected:
    void SetUp() override {
        description = "Test tool description";
        name = "test_tool";
        params = Json::Value(Json::objectValue);
        params["type"] = "object";
    }

    std::string description;
    std::string name;
    Json::Value params;
};

TEST_F(AssistantFunctionToolTest, BasicConstruction) {
    AssistantFunction function(description, name, params, true);
    AssistantFunctionTool tool(function);
    
    EXPECT_EQ(tool.type, "function");
    EXPECT_EQ(tool.function.description, description);
    EXPECT_EQ(tool.function.name, name);
    EXPECT_EQ(tool.function.parameters, params);
    ASSERT_TRUE(tool.function.strict.has_value());
    EXPECT_TRUE(*tool.function.strict);
}

TEST_F(AssistantFunctionToolTest, MoveConstructor) {
    AssistantFunction function(description, name, params, true);
    AssistantFunctionTool original(function);
    
    AssistantFunctionTool moved(std::move(original));
    
    EXPECT_EQ(moved.type, "function");
    EXPECT_EQ(moved.function.description, description);
    EXPECT_EQ(moved.function.name, name);
    EXPECT_EQ(moved.function.parameters, params);
}

TEST_F(AssistantFunctionToolTest, FromValidJson) {
    Json::Value function_json;
    function_json["description"] = description;
    function_json["name"] = name;
    function_json["strict"] = true;
    function_json["parameters"] = params;
    
    Json::Value json;
    json["type"] = "function";
    json["function"] = function_json;
    
    auto result = AssistantFunctionTool::FromJson(json);
    ASSERT_TRUE(result.has_value());
    
    const auto& tool = result.value();
    EXPECT_EQ(tool.type, "function");
    EXPECT_EQ(tool.function.description, description);
    EXPECT_EQ(tool.function.name, name);
    EXPECT_EQ(tool.function.parameters, params);
    ASSERT_TRUE(tool.function.strict.has_value());
    EXPECT_TRUE(*tool.function.strict);
}

TEST_F(AssistantFunctionToolTest, FromInvalidJson) {
    Json::Value json;
    auto result = AssistantFunctionTool::FromJson(json);
    EXPECT_TRUE(result.has_error());
    EXPECT_EQ(result.error(), "Failed to parse function: Function json can't be empty");
}

TEST_F(AssistantFunctionToolTest, ToJson) {
    AssistantFunction function(description, name, params, true);
    AssistantFunctionTool tool(function);
    
    auto result = tool.ToJson();
    ASSERT_TRUE(result.has_value());
    
    const auto& json = result.value();
    EXPECT_EQ(json["type"].asString(), "function");
    EXPECT_EQ(json["function"]["description"].asString(), description);
    EXPECT_EQ(json["function"]["name"].asString(), name);
    EXPECT_EQ(json["function"]["parameters"], params);
    EXPECT_TRUE(json["function"]["strict"].asBool());
}

}  // namespace
}  // namespace OpenAi
