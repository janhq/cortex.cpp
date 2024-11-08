#include <gtest/gtest.h>
#include <json/json.h>
#include "common/api_server_configuration.h"

class ApiServerConfigurationTest : public ::testing::Test {
 protected:
  ApiServerConfiguration config;

  // Helper to create JSON from string
  Json::Value parseJson(const std::string& jsonStr) {
    Json::Value root;
    Json::Reader reader;
    reader.parse(jsonStr, root);
    return root;
  }
};

// Test default values
TEST_F(ApiServerConfigurationTest, DefaultValues) {
  EXPECT_TRUE(config.cors);
  EXPECT_TRUE(config.allowed_origins.empty());
}

// Test CORS update
TEST_F(ApiServerConfigurationTest, UpdateCors) {
  auto json = parseJson(R"({"cors": false})");
  std::vector<std::string> updated_fields;
  config.UpdateFromJson(json, &updated_fields);

  EXPECT_FALSE(config.cors);
  ASSERT_EQ(updated_fields.size(), 1);
  EXPECT_EQ(updated_fields[0], "cors");
}

// Test allowed origins update
TEST_F(ApiServerConfigurationTest, UpdateAllowedOrigins) {
  auto json = parseJson(R"({
        "allowed_origins": ["https://example.com", "https://test.com"]
    })");
  std::vector<std::string> updated_fields;
  config.UpdateFromJson(json, &updated_fields);

  ASSERT_EQ(config.allowed_origins.size(), 2);
  EXPECT_EQ(config.allowed_origins[0], "https://example.com");
  EXPECT_EQ(config.allowed_origins[1], "https://test.com");
  ASSERT_EQ(updated_fields.size(), 1);
  EXPECT_EQ(updated_fields[0], "allowed_origins");
}

// Test multiple field updates
TEST_F(ApiServerConfigurationTest, UpdateMultipleFields) {
  auto json = parseJson(R"({
        "cors": false,
        "allowed_origins": ["https://example.com"]
    })");
  std::vector<std::string> updated_fields;
  config.UpdateFromJson(json, &updated_fields);

  EXPECT_FALSE(config.cors);
  ASSERT_EQ(config.allowed_origins.size(), 1);
  EXPECT_EQ(config.allowed_origins[0], "https://example.com");
  ASSERT_EQ(updated_fields.size(), 2);
}

// Test unknown fields
TEST_F(ApiServerConfigurationTest, UnknownFields) {
  auto json = parseJson(R"({
        "cors": false,
        "unknown_field": "value"
    })");
  std::vector<std::string> updated_fields;
  std::vector<std::string> unknown_fields;
  config.UpdateFromJson(json, &updated_fields, nullptr, &unknown_fields);

  EXPECT_FALSE(config.cors);
  ASSERT_EQ(updated_fields.size(), 1);
  ASSERT_EQ(unknown_fields.size(), 1);
  EXPECT_EQ(unknown_fields[0], "unknown_field");
}

// Test invalid field types
TEST_F(ApiServerConfigurationTest, InvalidFieldTypes) {
  auto json = parseJson(R"({
        "cors": "invalid_bool",
        "allowed_origins": "invalid_array"
    })");
  std::vector<std::string> updated_fields;
  std::vector<std::string> invalid_fields;
  config.UpdateFromJson(json, &updated_fields, &invalid_fields);

  for (const auto& field : updated_fields) {
    std::cout << field << std::endl;
  }

  EXPECT_TRUE(config.cors);                     // Should retain default value
  EXPECT_TRUE(config.allowed_origins.empty());  // Should retain default value
  EXPECT_TRUE(updated_fields.empty());
}

// Test empty update
TEST_F(ApiServerConfigurationTest, EmptyUpdate) {
  auto json = parseJson("{}");
  std::vector<std::string> updated_fields;
  config.UpdateFromJson(json, &updated_fields);

  EXPECT_TRUE(config.cors);  // Should retain default value
  EXPECT_TRUE(config.allowed_origins.empty());
  EXPECT_TRUE(updated_fields.empty());
}

// Test allowed_origins with invalid array elements
TEST_F(ApiServerConfigurationTest, InvalidArrayElements) {
  auto json = parseJson(R"({
        "allowed_origins": ["valid", 123, true, "also_valid"]
    })");
  std::vector<std::string> updated_fields;
  config.UpdateFromJson(json, &updated_fields);

  ASSERT_EQ(updated_fields.size(), 0);
  ASSERT_EQ(config.allowed_origins.size(), 0);
}

// Test nullopt parameters
TEST_F(ApiServerConfigurationTest, NulloptParameters) {
  auto json = parseJson(R"({
        "cors": false,
        "unknown_field": "value"
    })");
  config.UpdateFromJson(json);  // Should not crash

  EXPECT_FALSE(config.cors);
}
