#include "gtest/gtest.h"
#include "utils/string_utils.h"

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
