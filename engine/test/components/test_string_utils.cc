#include <cmath>
#include <sstream>
#include "gtest/gtest.h"
#include "utils/string_utils.h"
#include "utils/string_utils.h"  // Assuming the original code is in this header file
class StringUtilsTestSuite : public ::testing::Test {};

TEST_F(StringUtilsTestSuite, TestSplitBy) {
  auto input = "this is a test";
  std::string delimiter{' '};
  auto result = string_utils::SplitBy(input, delimiter);

  EXPECT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], "this");
  EXPECT_EQ(result[1], "is");
  EXPECT_EQ(result[2], "a");
  EXPECT_EQ(result[3], "test");
}

TEST_F(StringUtilsTestSuite, TestSplitByWithEmptyString) {
  auto input = "";
  std::string delimiter{' '};
  auto result = string_utils::SplitBy(input, delimiter);

  EXPECT_EQ(result.size(), 0);
}

TEST_F(StringUtilsTestSuite, TestSplitModelHandle) {
  auto input = "cortexso/tinyllama";
  std::string delimiter{'/'};
  auto result = string_utils::SplitBy(input, delimiter);

  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], "cortexso");
  EXPECT_EQ(result[1], "tinyllama");
}

TEST_F(StringUtilsTestSuite, TestSplitModelHandleWithEmptyModelName) {
  auto input = "cortexso/";
  std::string delimiter{'/'};
  auto result = string_utils::SplitBy(input, delimiter);

  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], "cortexso");
}

TEST_F(StringUtilsTestSuite, TestStartsWith) {
  auto input = "this is a test";
  auto prefix = "this";
  EXPECT_TRUE(string_utils::StartsWith(input, prefix));
}

TEST_F(StringUtilsTestSuite, TestStartsWithWithEmptyString) {
  auto input = "";
  auto prefix = "this";
  EXPECT_FALSE(string_utils::StartsWith(input, prefix));
}

TEST_F(StringUtilsTestSuite, TestStartsWithWithEmptyPrefix) {
  auto input = "this is a test";
  auto prefix = "";
  EXPECT_TRUE(string_utils::StartsWith(input, prefix));
}

TEST_F(StringUtilsTestSuite, TestEndsWith) {
  auto input = "this is a test";
  auto suffix = "test";
  EXPECT_TRUE(string_utils::EndsWith(input, suffix));
}

TEST_F(StringUtilsTestSuite, TestEndsWithWithEmptyString) {
  auto input = "";
  auto suffix = "test";
  EXPECT_FALSE(string_utils::EndsWith(input, suffix));
}

TEST_F(StringUtilsTestSuite, TestEndsWithWithEmptySuffix) {
  auto input = "this is a test";
  auto suffix = "";
  EXPECT_TRUE(string_utils::EndsWith(input, suffix));
}

TEST_F(StringUtilsTestSuite, PrintComment) {
  std::string result = string_utils::print_comment("Test comment");
  EXPECT_EQ(result, "\033[1;90m# Test comment\033[0m\n");
}

TEST_F(StringUtilsTestSuite, PrintKV) {
  std::string result = string_utils::print_kv("key", "value");
  EXPECT_EQ(result, "\033[1;32mkey:\033[0m \033[0mvalue\033[0m\n");

  std::string result2 = string_utils::print_kv("key", "value", "\033[0;34m");
  EXPECT_EQ(result2, "\033[1;32mkey:\033[0m \033[0;34mvalue\033[0m\n");
}

TEST_F(StringUtilsTestSuite, PrintBool) {
  std::string result = string_utils::print_bool("key", true);
  EXPECT_EQ(result, "\033[1;32mkey:\033[0m \033[0;35mtrue\033[0m\n");

  result = string_utils::print_bool("key", false);
  EXPECT_EQ(result, "\033[1;32mkey:\033[0m \033[0;35mfalse\033[0m\n");
}

TEST_F(StringUtilsTestSuite, PrintFloat) {
  std::string result = string_utils::print_float("key", 3.14159f);
  EXPECT_EQ(result, "\033[1;32mkey:\033[0m \033[0;34m3.14159\033[0m\n");

  result = string_utils::print_float("key", 3.000f);
  EXPECT_EQ(result, "\033[1;32mkey:\033[0m \033[0;34m3\033[0m\n");

  result = string_utils::print_float("key", 3.14000f);
  EXPECT_EQ(result, "\033[1;32mkey:\033[0m \033[0;34m3.14\033[0m\n");

  result = string_utils::print_float("key", std::numeric_limits<float>::quiet_NaN());
  EXPECT_EQ(result, "");
}