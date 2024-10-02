#include <yaml-cpp/yaml.h>
#include <limits>
#include "gtest/gtest.h"
#include "utils/format_utils.h"

class FormatUtilsTest : public ::testing::Test {};

TEST_F(FormatUtilsTest, WriteKeyValue) {
  {
    YAML::Node node;
    std::string result = format_utils::writeKeyValue("key", node["does_not_exist"]);
    EXPECT_EQ(result, "");
  }

  {
    YAML::Node node = YAML::Load("value");
    std::string result = format_utils::writeKeyValue("key", node);
    EXPECT_EQ(result, "key: value\n");
  }

  {
    YAML::Node node = YAML::Load("3.14159");
    std::string result = format_utils::writeKeyValue("key", node);
    EXPECT_EQ(result, "key: 3.14159\n");
  }

  {
    YAML::Node node = YAML::Load("3.000000");
    std::string result = format_utils::writeKeyValue("key", node);
    EXPECT_EQ(result, "key: 3\n");
  }

  {
    YAML::Node node = YAML::Load("3.140000");
    std::string result = format_utils::writeKeyValue("key", node);
    EXPECT_EQ(result, "key: 3.14\n");
  }

  {
    YAML::Node node = YAML::Load("value");
    std::string result = format_utils::writeKeyValue("key", node, "comment");
    EXPECT_EQ(result, "key: value # comment\n");
  }
}

TEST_F(FormatUtilsTest, BytesToHumanReadable) {
  {
    uint64_t bytes = 500;
    std::string result = format_utils::BytesToHumanReadable(bytes);
    EXPECT_EQ(result, "500.00 B");
  }

  {
    uint64_t bytes = 1500;
    std::string result = format_utils::BytesToHumanReadable(bytes);
    EXPECT_EQ(result, "1.46 KB");
  }

  {
    uint64_t bytes = 1500000;
    std::string result = format_utils::BytesToHumanReadable(bytes);
    EXPECT_EQ(result, "1.43 MB");
  }

  {
    uint64_t bytes = 1500000000;
    std::string result = format_utils::BytesToHumanReadable(bytes);
    EXPECT_EQ(result, "1.40 GB");
  }

  {
    uint64_t bytes = 1500000000000;
    std::string result = format_utils::BytesToHumanReadable(bytes);
    EXPECT_EQ(result, "1.36 TB");
  }
}