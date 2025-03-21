#include "gtest/gtest.h"
#include "utils/string_utils.h"

class StringUtilsTestSuite : public ::testing::Test {};
using namespace string_utils;

TEST_F(StringUtilsTestSuite, ParsePrompt) {
  {
    std::string prompt =
        "<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\n{"
        "system_message}<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n{"
        "prompt}<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n";
    auto result = ParsePrompt(prompt);
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
  auto result = SplitBy(input, " ");

  EXPECT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], "this");
  EXPECT_EQ(result[1], "is");
  EXPECT_EQ(result[2], "a");
  EXPECT_EQ(result[3], "test");
}

TEST_F(StringUtilsTestSuite, TestSplitByWithEmptyString) {
  auto input = "";
  auto result = SplitBy(input, " ");

  EXPECT_EQ(result.size(), 0);
}

TEST_F(StringUtilsTestSuite, TestSplitModelHandle) {
  auto input = "cortexso/tinyllama";
  auto result = SplitBy(input, "/");

  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], "cortexso");
  EXPECT_EQ(result[1], "tinyllama");
}

TEST_F(StringUtilsTestSuite, TestSplitModelHandleWithEmptyModelName) {
  auto input = "cortexso/";
  auto result = SplitBy(input, "/");

  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], "cortexso");
}

TEST_F(StringUtilsTestSuite, TestSplitIfNotContainDelimiter) {
  auto input = "https://cortex.so";
  auto result = SplitBy(input, ",");

  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], "https://cortex.so");
}

TEST_F(StringUtilsTestSuite, TestStartsWith) {
  auto input = "this is a test";
  auto prefix = "this";
  EXPECT_TRUE(StartsWith(input, prefix));
}

TEST_F(StringUtilsTestSuite, TestStartsWithWithEmptyString) {
  auto input = "";
  auto prefix = "this";
  EXPECT_FALSE(StartsWith(input, prefix));
}

TEST_F(StringUtilsTestSuite, TestStartsWithWithEmptyPrefix) {
  auto input = "this is a test";
  auto prefix = "";
  EXPECT_TRUE(StartsWith(input, prefix));
}

TEST_F(StringUtilsTestSuite, TestEndsWith) {
  auto input = "this is a test";
  auto suffix = "test";
  EXPECT_TRUE(EndsWith(input, suffix));
}

TEST_F(StringUtilsTestSuite, TestEndsWithWithEmptyString) {
  auto input = "";
  auto suffix = "test";
  EXPECT_FALSE(EndsWith(input, suffix));
}

TEST_F(StringUtilsTestSuite, TestEndsWithWithEmptySuffix) {
  auto input = "this is a test";
  auto suffix = "";
  EXPECT_TRUE(EndsWith(input, suffix));
}

TEST_F(StringUtilsTestSuite, EmptyString) {
  std::string s = "";
  Trim(s);
  EXPECT_EQ(s, "");
}

TEST_F(StringUtilsTestSuite, NoWhitespace) {
  std::string s = "hello";
  Trim(s);
  EXPECT_EQ(s, "hello");
}

TEST_F(StringUtilsTestSuite, LeadingWhitespace) {
  std::string s = "   hello";
  Trim(s);
  EXPECT_EQ(s, "hello");
}

TEST_F(StringUtilsTestSuite, TrailingWhitespace) {
  std::string s = "hello   ";
  Trim(s);
  EXPECT_EQ(s, "hello");
}

TEST_F(StringUtilsTestSuite, BothEndsWhitespace) {
  std::string s = "   hello   ";
  Trim(s);
  EXPECT_EQ(s, "hello");
}

TEST_F(StringUtilsTestSuite, ExitString) {
  std::string s = "exit()   ";
  Trim(s);
  EXPECT_EQ(s, "exit()");
}

TEST_F(StringUtilsTestSuite, AllWhitespace) {
  std::string s = "     ";
  Trim(s);
  EXPECT_EQ(s, "");
}

TEST_F(StringUtilsTestSuite, MixedWhitespace) {
  std::string s = " \t\n  hello world \r\n ";
  Trim(s);
  EXPECT_EQ(s, "hello world");
}

TEST_F(StringUtilsTestSuite, EqualStrings) {
  EXPECT_TRUE(EqualsIgnoreCase("hello", "hello"));
  EXPECT_TRUE(EqualsIgnoreCase("WORLD", "WORLD"));
}

TEST_F(StringUtilsTestSuite, DifferentCaseStrings) {
  EXPECT_TRUE(EqualsIgnoreCase("Hello", "hElLo"));
  EXPECT_TRUE(EqualsIgnoreCase("WORLD", "world"));
  EXPECT_TRUE(EqualsIgnoreCase("MiXeD", "mIxEd"));
}

TEST_F(StringUtilsTestSuite, EmptyStrings) {
  EXPECT_TRUE(EqualsIgnoreCase("", ""));
}

TEST_F(StringUtilsTestSuite, DifferentStrings) {
  EXPECT_FALSE(EqualsIgnoreCase("hello", "world"));
  EXPECT_FALSE(EqualsIgnoreCase("HELLO", "hello world"));
}

TEST_F(StringUtilsTestSuite, DifferentLengthStrings) {
  EXPECT_FALSE(EqualsIgnoreCase("short", "longer string"));
  EXPECT_FALSE(EqualsIgnoreCase("LONG STRING", "long"));
}

TEST_F(StringUtilsTestSuite, SpecialCharacters) {
  EXPECT_TRUE(EqualsIgnoreCase("Hello!", "hElLo!"));
  EXPECT_TRUE(EqualsIgnoreCase("123 ABC", "123 abc"));
  EXPECT_FALSE(EqualsIgnoreCase("Hello!", "Hello"));
}

TEST_F(StringUtilsTestSuite, BasicMatching) {
  EXPECT_TRUE(StringContainsIgnoreCase("Hello, World!", "world"));
  EXPECT_TRUE(StringContainsIgnoreCase("Hello, World!", "Hello"));
  EXPECT_TRUE(StringContainsIgnoreCase("Hello, World!", "lo, wo"));
}

TEST_F(StringUtilsTestSuite, CaseSensitivity) {
  EXPECT_TRUE(StringContainsIgnoreCase("HELLO", "hello"));
  EXPECT_TRUE(StringContainsIgnoreCase("hello", "HELLO"));
  EXPECT_TRUE(StringContainsIgnoreCase("HeLLo", "ELL"));
}

TEST_F(StringUtilsTestSuite, EdgeCases) {
  EXPECT_TRUE(StringContainsIgnoreCase("", ""));
  EXPECT_TRUE(StringContainsIgnoreCase("Hello", ""));
  EXPECT_FALSE(StringContainsIgnoreCase("", "Hello"));
}

TEST_F(StringUtilsTestSuite, NoMatch) {
  EXPECT_FALSE(StringContainsIgnoreCase("Hello, World!", "Goodbye"));
  EXPECT_FALSE(StringContainsIgnoreCase("Hello", "HelloWorld"));
}

TEST_F(StringUtilsTestSuite, StringContainsWithSpecialCharacters) {
  EXPECT_TRUE(StringContainsIgnoreCase("Hello, World!", "o, W"));
  EXPECT_TRUE(StringContainsIgnoreCase("Hello! @#$%", "@#$"));
}

TEST_F(StringUtilsTestSuite, StringContainsWithModelId) {
  EXPECT_TRUE(StringContainsIgnoreCase(
      "TheBloke:TinyLlama-1.1B-Chat-v0.3-GGUF:tinyllama-1.1b-chat-v0.3.Q2_K."
      "gguf",
      "thebloke"));
}

TEST_F(StringUtilsTestSuite, RepeatingPatterns) {
  EXPECT_TRUE(StringContainsIgnoreCase("Mississippi", "ssi"));
  EXPECT_TRUE(StringContainsIgnoreCase("Mississippi", "ssippi"));
}

TEST_F(StringUtilsTestSuite, LongStrings) {
  EXPECT_TRUE(
      StringContainsIgnoreCase("This is a very long string to test our "
                               "function's performance with larger inputs",
                               "PERFORMANCE"));
  EXPECT_FALSE(
      StringContainsIgnoreCase("This is a very long string to test our "
                               "function's performance with larger inputs",
                               "not here"));
}

TEST_F(StringUtilsTestSuite, BasicRemoval) {
  EXPECT_EQ(RemoveSubstring("hello world", "o"), "hell wrld");
  EXPECT_EQ(RemoveSubstring("hello world", "l"), "heo word");
}

TEST_F(StringUtilsTestSuite, MultipleOccurrences) {
  EXPECT_EQ(RemoveSubstring("banana", "a"), "bnn");
  EXPECT_EQ(RemoveSubstring("hello hello", "hello"), " ");
}

TEST_F(StringUtilsTestSuite, NoOccurrences) {
  EXPECT_EQ(RemoveSubstring("hello world", "x"), "hello world");
  EXPECT_EQ(RemoveSubstring("test", "xyz"), "test");
}

TEST_F(StringUtilsTestSuite, RemoveEmptyStrings) {
  EXPECT_EQ(RemoveSubstring("", ""), "");
  EXPECT_EQ(RemoveSubstring("hello", ""), "hello");
  EXPECT_EQ(RemoveSubstring("", "hello"), "");
}

TEST_F(StringUtilsTestSuite, EntireStringMatch) {
  EXPECT_EQ(RemoveSubstring("hello", "hello"), "");
  EXPECT_EQ(RemoveSubstring("test", "test"), "");
}

TEST_F(StringUtilsTestSuite, OverlappingPatterns) {
  EXPECT_EQ(RemoveSubstring("aaaa", "aa"), "");  // Should remove "aa" twice
  EXPECT_EQ(RemoveSubstring("aaa", "aa"), "a");  // Should remove first "aa"
}

TEST_F(StringUtilsTestSuite, RemoveSubstringCaseSensitivity) {
  EXPECT_EQ(RemoveSubstring("Hello World", "hello"), "Hello World");
  EXPECT_EQ(RemoveSubstring("Hello World", "Hello"), " World");
}

TEST_F(StringUtilsTestSuite, RemoveSubstringSpecialCharacters) {
  EXPECT_EQ(RemoveSubstring("hello\nworld", "\n"), "helloworld");
  EXPECT_EQ(RemoveSubstring("hello\tworld", "\t"), "helloworld");
  EXPECT_EQ(RemoveSubstring("hello  world", "  "), "helloworld");
}

TEST_F(StringUtilsTestSuite, RemoveSubstringLongStrings) {
  std::string long_string(1000, 'a');  // String of 1000 'a' characters
  std::string expected("");            // String of 900 'a' characters
  EXPECT_EQ(RemoveSubstring(long_string, std::string(100, 'a')), expected);
}

// Performance test (optional, might want to move to a benchmark suite)
TEST_F(StringUtilsTestSuite, LargeInputPerformance) {
  std::string large_input = std::string(1000000, 'x');  // 1M characters
  std::string to_remove = "x";

  // This test mainly ensures the function completes in a reasonable time
  // and doesn't crash with large inputs
  EXPECT_EQ(RemoveSubstring(large_input, to_remove), "");
}

TEST(LTrimTest, EmptyString) {
  std::string s = "";
  LTrim(s);
  EXPECT_EQ(s, "");
}

TEST(LTrimTest, NoSpaces) {
  std::string s = "HelloWorld";
  LTrim(s);
  EXPECT_EQ(s, "HelloWorld");
}

TEST(LTrimTest, LeadingSpaces) {
  std::string s = "   HelloWorld";
  LTrim(s);
  EXPECT_EQ(s, "HelloWorld");
}

TEST(LTrimTest, LeadingTabs) {
  std::string s = "\t\tHelloWorld";
  LTrim(s);
  EXPECT_EQ(s, "HelloWorld");
}

TEST(LTrimTest, LeadingNewlines) {
  std::string s = "\n\nHelloWorld";
  LTrim(s);
  EXPECT_EQ(s, "HelloWorld");
}

TEST(LTrimTest, OnlySpaces) {
  std::string s = "   ";
  LTrim(s);
  EXPECT_EQ(s, "");
}

TEST(LTrimTest, MixedSpaces) {
  std::string s = "   \t\nHelloWorld   ";
  LTrim(s);
  EXPECT_EQ(s, "HelloWorld   ");
}
