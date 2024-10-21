#include "gtest/gtest.h"
#include "utils/string_utils.h"

class StringUtilsTestSuite : public ::testing::Test {};

TEST_F(StringUtilsTestSuite, ParsePrompt) {
  {
    std::string prompt =
        "<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\n{"
        "system_message}<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n{"
        "prompt}<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
    auto result = string_utils::ParsePrompt(prompt);
    EXPECT_EQ(result.user_prompt,
              "<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n");
    EXPECT_EQ(result.ai_prompt,
              "<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n");
    EXPECT_EQ(
        result.system_prompt,
        "<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\n");
  }
}

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

TEST_F(StringUtilsTestSuite, EmptyString) {
  std::string s = "";
  string_utils::Trim(s);
  EXPECT_EQ(s, "");
}

TEST_F(StringUtilsTestSuite, NoWhitespace) {
  std::string s = "hello";
  string_utils::Trim(s);
  EXPECT_EQ(s, "hello");
}

TEST_F(StringUtilsTestSuite, LeadingWhitespace) {
  std::string s = "   hello";
  string_utils::Trim(s);
  EXPECT_EQ(s, "hello");
}

TEST_F(StringUtilsTestSuite, TrailingWhitespace) {
  std::string s = "hello   ";
  string_utils::Trim(s);
  EXPECT_EQ(s, "hello");
}

TEST_F(StringUtilsTestSuite, BothEndsWhitespace) {
  std::string s = "   hello   ";
  string_utils::Trim(s);
  EXPECT_EQ(s, "hello");
}

TEST_F(StringUtilsTestSuite, ExitString) {
  std::string s = "exit()   ";
  string_utils::Trim(s);
  EXPECT_EQ(s, "exit()");
}

TEST_F(StringUtilsTestSuite, AllWhitespace) {
  std::string s = "     ";
  string_utils::Trim(s);
  EXPECT_EQ(s, "");
}

TEST_F(StringUtilsTestSuite, MixedWhitespace) {
  std::string s = " \t\n  hello world \r\n ";
  string_utils::Trim(s);
  EXPECT_EQ(s, "hello world");
}

TEST_F(StringUtilsTestSuite, EqualStrings) {
  EXPECT_TRUE(string_utils::EqualsIgnoreCase("hello", "hello"));
  EXPECT_TRUE(string_utils::EqualsIgnoreCase("WORLD", "WORLD"));
}

TEST_F(StringUtilsTestSuite, DifferentCaseStrings) {
  EXPECT_TRUE(string_utils::EqualsIgnoreCase("Hello", "hElLo"));
  EXPECT_TRUE(string_utils::EqualsIgnoreCase("WORLD", "world"));
  EXPECT_TRUE(string_utils::EqualsIgnoreCase("MiXeD", "mIxEd"));
}

TEST_F(StringUtilsTestSuite, EmptyStrings) {
  EXPECT_TRUE(string_utils::EqualsIgnoreCase("", ""));
}

TEST_F(StringUtilsTestSuite, DifferentStrings) {
  EXPECT_FALSE(string_utils::EqualsIgnoreCase("hello", "world"));
  EXPECT_FALSE(string_utils::EqualsIgnoreCase("HELLO", "hello world"));
}

TEST_F(StringUtilsTestSuite, DifferentLengthStrings) {
  EXPECT_FALSE(string_utils::EqualsIgnoreCase("short", "longer string"));
  EXPECT_FALSE(string_utils::EqualsIgnoreCase("LONG STRING", "long"));
}

TEST_F(StringUtilsTestSuite, SpecialCharacters) {
  EXPECT_TRUE(string_utils::EqualsIgnoreCase("Hello!", "hElLo!"));
  EXPECT_TRUE(string_utils::EqualsIgnoreCase("123 ABC", "123 abc"));
  EXPECT_FALSE(string_utils::EqualsIgnoreCase("Hello!", "Hello"));
}

TEST_F(StringUtilsTestSuite, BasicMatching) {
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("Hello, World!", "world"));
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("Hello, World!", "Hello"));
  EXPECT_TRUE(
      string_utils::StringContainsIgnoreCase("Hello, World!", "lo, wo"));
}

TEST_F(StringUtilsTestSuite, CaseSensitivity) {
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("HELLO", "hello"));
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("hello", "HELLO"));
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("HeLLo", "ELL"));
}

TEST_F(StringUtilsTestSuite, EdgeCases) {
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("", ""));
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("Hello", ""));
  EXPECT_FALSE(string_utils::StringContainsIgnoreCase("", "Hello"));
}

TEST_F(StringUtilsTestSuite, NoMatch) {
  EXPECT_FALSE(
      string_utils::StringContainsIgnoreCase("Hello, World!", "Goodbye"));
  EXPECT_FALSE(string_utils::StringContainsIgnoreCase("Hello", "HelloWorld"));
}

TEST_F(StringUtilsTestSuite, StringContainsWithSpecialCharacters) {
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("Hello, World!", "o, W"));
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("Hello! @#$%", "@#$"));
}

TEST_F(StringUtilsTestSuite, StringContainsWithModelId) {
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase(
      "TheBloke:TinyLlama-1.1B-Chat-v0.3-GGUF:tinyllama-1.1b-chat-v0.3.Q2_K."
      "gguf",
      "thebloke"));
}

TEST_F(StringUtilsTestSuite, RepeatingPatterns) {
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("Mississippi", "ssi"));
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase("Mississippi", "ssippi"));
}

TEST_F(StringUtilsTestSuite, LongStrings) {
  EXPECT_TRUE(string_utils::StringContainsIgnoreCase(
      "This is a very long string to test our "
      "function's performance with larger inputs",
      "PERFORMANCE"));
  EXPECT_FALSE(string_utils::StringContainsIgnoreCase(
      "This is a very long string to test our "
      "function's performance with larger inputs",
      "not here"));
}
