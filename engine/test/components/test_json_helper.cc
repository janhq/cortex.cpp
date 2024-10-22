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
