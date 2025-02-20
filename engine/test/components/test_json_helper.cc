#include <gtest/gtest.h>
#include <json/json.h>
#include <string>
#include "utils/json_helper.h"

// Test Suite
TEST(ParseJsonStringTest, ValidJsonObject) {
    std::string json_str = R"({"name": "John", "age": 30})";
    Json::Value result = json_helper::ParseJsonString(json_str);
    
    EXPECT_EQ(result["name"].asString(), "John");
    EXPECT_EQ(result["age"].asInt(), 30);
}

TEST(ParseJsonStringTest, ValidJsonArray) {
    std::string json_str = R"([1, 2, 3, 4, 5])";
    Json::Value result = json_helper::ParseJsonString(json_str);
    
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0].asInt(), 1);
}

TEST(ParseJsonStringTest, InvalidJson) {
    std::string json_str = R"({"name": "John", "age": )"; 
    Json::Value result = json_helper::ParseJsonString(json_str);
    
    EXPECT_TRUE(result["age"].isNull());
}

TEST(ParseJsonStringTest, EmptyString) {
    std::string json_str = "";
    Json::Value result = json_helper::ParseJsonString(json_str);
    
    EXPECT_TRUE(result.isNull()); 
}

TEST(MergeJsonTest, MergeSimpleObjects) {
    Json::Value json1, json2;
    json1["name"] = "John";
    json1["age"] = 30;

    json2["age"] = 31;
    json2["email"] = "john@example.com";

    json_helper::MergeJson(json1, json2);

    Json::Value expected;
    expected["name"] = "John";
    expected["age"] = 31;
    expected["email"] = "john@example.com";

    EXPECT_EQ(json1, expected);
}

TEST(MergeJsonTest, MergeNestedObjects) {
    Json::Value json1, json2;
    json1["person"]["name"] = "John";
    json1["person"]["age"] = 30;

    json2["person"]["age"] = 31;
    json2["person"]["email"] = "john@example.com";

    json_helper::MergeJson(json1, json2);

    Json::Value expected;
    expected["person"]["name"] = "John";
    expected["person"]["age"] = 31;
    expected["person"]["email"] = "john@example.com";

    EXPECT_EQ(json1, expected);
}

TEST(MergeJsonTest, MergeArrays) {
    Json::Value json1, json2;
    json1["hobbies"] = Json::Value(Json::arrayValue);
    json1["hobbies"].append("reading");
    json1["hobbies"].append("painting");

    json2["hobbies"] = Json::Value(Json::arrayValue);
    json2["hobbies"].append("hiking");
    json2["hobbies"].append("painting");

    json_helper::MergeJson(json1, json2);

    Json::Value expected;
    expected["hobbies"] = Json::Value(Json::arrayValue);
    expected["hobbies"].append("reading");
    expected["hobbies"].append("painting");
    expected["hobbies"].append("hiking");
    expected["hobbies"].append("painting");

    EXPECT_EQ(json1, expected);
}
